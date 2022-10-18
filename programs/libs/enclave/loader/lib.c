#include <fcntl.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

// elf64 library includes.
#include "elf64.h"
// local includes.
#include "encl_loader.h"
#include "lib_internal.h"

const char* STACK_SECTION_NAME = ".encl_stack";
const char* ENCL_DRIVER = "/dev/tyche_enclave"; 

static lib_encl_t* library_plugin = NULL;

static void* mmap_file(const char* file, int* fd)
{
  // First open the file.
  *fd = open(file, O_RDONLY);
  if (*fd < 0) {
    goto fail;
  }

  // Now mmap the file.
  struct stat s;
  int status = fstat(*fd, &s);
  if (status < 0) {
    goto fail_close;
  } 
  void* ptr = mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, *fd, 0); 
  if (ptr == MAP_FAILED) {
    goto fail_close;
  }
  // Success.
  return ptr;

  // Failure.
fail_close:
  close(*fd);
fail:
  return NULL;
}

/// Parses the enclave's ELF.
static int parse_elf(load_encl_t* encl)
{
  if (encl == NULL || encl->elf_fd < 0) {
    return -1;
  }
  // Header
  read_elf64_header(encl->elf_fd, &(encl->header));
  
  // Segments to map.
  read_elf64_segments(encl->elf_fd, encl->header, &(encl->segments));
  
  // Sections: find the stack.
  // Later we can use that part to divide kernel vs. user code too.
  read_elf64_sections(encl->elf_fd, encl->header, &(encl->sections));
  char *sh_names = read_section64(encl->elf_fd, encl->sections[encl->header.e_shstrndx]); 
  for (int i = 0; i < encl->header.e_shnum; i++) {
    if (strcmp(sh_names + encl->sections[i].sh_name, STACK_SECTION_NAME) == 0) {
      encl->stack_section = &(encl->sections[i]);
      break;
    } 
  }
  free(sh_names);
  sh_names = NULL;
  // Do this the clean way. For now, let's just have a variable. 
  if(encl->stack_section == NULL) {
    fprintf(stderr, "[encl_loader]: no stack section.\n");
    goto fail;
  }
  // All went well
  return 0;
fail:
  free(encl->sections);
  free(encl->segments);
  return -1;
}

static uint64_t translate_elf_flags(Elf64_Word flags) {
  uint64_t result = TE_USER;
  if (flags & PF_X) {
    result |= TE_EXEC; 
  }
  if (flags & PF_W) {
    result |= TE_WRITE;
  }
  if (flags & PF_R) {
    result |= TE_READ;
  }
  return result;
}

static int create_enclave(load_encl_t* enclave, struct tyche_encl_add_region_t* extras)
{
  enclave->driver_fd = open(ENCL_DRIVER, O_RDWR);
  if (enclave->driver_fd < 0) {
    goto fail;
  }

  // Create the enclave.
  struct tyche_encl_create_t create;
  if (ioctl(enclave->driver_fd, TYCHE_ENCLAVE_CREATE, &create) == -1) {
    goto fail_close;
  }
  // Set the handle.
  enclave->handle = create.handle;
 
  // Allocate the tracker for each segment mapping.
  enclave->mappings = calloc(sizeof(void*), enclave->header.e_phnum);
  if (enclave->mappings == NULL) {
    goto fail_close;
  }
  enclave->sizes = calloc(sizeof(size_t), enclave->header.e_phnum);
  if (enclave->sizes == NULL) {
    goto fail_free;
  }
  // Set all the mappings to NULL.
  for (int i = 0; i < enclave->header.e_phnum; i++) {
    enclave->mappings[i] = NULL;
  }

  // Add the encl.so to the enclave.
  do {
    struct tyche_encl_add_region_t region = {
      .handle = enclave->handle,
      .start = (uint64_t)library_plugin->plugin,
      .end = ((uint64_t)(library_plugin->plugin)) + library_plugin->size,
      .src = (uint64_t)library_plugin->plugin,
      .flags = TE_READ|TE_EXEC|TE_USER,
      .tpe = Shared,
    };

    if (ioctl(enclave->driver_fd, TYCHE_ENCLAVE_ADD_REGION, &region) != 0) {
      goto fail_free;
    }

  } while(0);
  
  // Add the extras too.
  do {
    struct tyche_encl_add_region_t* curr = NULL;
    for (curr = extras; curr != NULL; curr = (struct tyche_encl_add_region_t*)curr->extra) {
      if (ioctl(enclave->driver_fd, TYCHE_ENCLAVE_ADD_REGION, curr) !=0) {
        goto fail_free;
      }
    }
  } while(0);
  

  // Load each segment.
  for (int i = 0; i < enclave->header.e_phnum; i++) {
    Elf64_Phdr segment = enclave->segments[i]; 
    uint64_t flags = 0;

    // Non-loadable segments are ignored.
    if (segment.p_type != PT_LOAD) {
      continue;
    } 

    // Align up if needed.
    size_t modulo = (size_t) (segment.p_memsz % segment.p_align);
    enclave->sizes[i] = segment.p_memsz + (modulo != 0) * (segment.p_align - modulo); 
    enclave->mappings[i] = mmap(NULL, enclave->sizes[i], PROT_READ|PROT_WRITE, 
        MAP_PRIVATE|MAP_POPULATE|MAP_ANONYMOUS, -1, 0);
    if (enclave->mappings[i] == MAP_FAILED) {
      goto fail_unmap;
    }

    // Copy the content from file  + offset.
    memcpy(enclave->mappings[i],
        enclave->elf_content + segment.p_offset, segment.p_filesz);
    
    // Translate the flags.
    flags = translate_elf_flags(segment.p_flags);
    struct tyche_encl_add_region_t region = {
      .handle = enclave->handle,
      .start = segment.p_vaddr, //TODO this will depend on the elf type.
      .end = segment.p_vaddr + enclave->sizes[i],
      .src = (uint64_t)enclave->mappings[i],
      .flags = flags,
      .tpe = Confidential,
    };
    // Call the driver with segment.p_vaddr, p_vaddr + pmemsz, p_flags
    if(ioctl(enclave->driver_fd, TYCHE_ENCLAVE_ADD_REGION, &region) != 0) {
      goto fail_unmap;
    }
  }
  // Everything went well.
  return 0;

  // Errors.
fail_unmap:
  for (int i = 0; i < enclave->header.e_phnum; i++) {
    if (enclave->mappings[i] == NULL || enclave->mappings[i] == MAP_FAILED) {
      continue;
    }
    munmap(enclave->mappings[i], enclave->sizes[i]);
  }
  free(enclave->sizes);
fail_free:
  free(enclave->mappings);
fail_close:
  close(enclave->driver_fd);
fail:
  return -1;
}

//TODO change this, replace with a mapping of the lib.
const lib_encl_t* init_enclave_loader(const char* libencl)
{
  if (library_plugin != NULL) {
    return library_plugin;
  }
  int fd = open(libencl, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "[encl_loader]: Unable to open libencl '%s'.\n", libencl);
    goto fail;
  }
  Elf64_Ehdr header; 
  read_elf64_header(fd, &header);

  Elf64_Phdr* segments = NULL;
  read_elf64_segments(fd, header, &segments);
  
  // Find the .text section.
  Elf64_Shdr* sections = NULL;
  read_elf64_sections(fd, header, &sections);
  char* sh_names = read_section64(fd, sections[header.e_shstrndx]); 
  Elf64_Shdr* text = NULL;
  int text_idx = -1;
  Elf64_Phdr* text_seg = NULL;
  for (int i = 0; i < header.e_shnum; i++) {
    if (strcmp(".text", sh_names + sections[i].sh_name) == 0) {
      text = &sections[i];
      text_idx = i;
      break;
    }
  }
  free(sh_names);
  if (!text)
  {
    fprintf(stderr, "[encl_loader]: unable to find text section in libencl.\n");
    goto fail_close;
  }

  // Find the correct segment.
  for (int i = 0; i < header.e_phnum; i++) {
    if (segments[i].p_offset <= text->sh_offset 
        && ((segments[i].p_offset +segments[i].p_filesz) >= text->sh_offset + text->sh_size)) {
      text_seg = &segments[i];
      break;
    }
  } 
  if(text_seg == NULL) {
    fprintf(stderr, "[encl_loader]: no text segment found.\n");
    goto fail_close;
  }
  
  library_plugin = malloc(sizeof(lib_encl_t));
  if (!library_plugin) {
    fprintf(stderr, "[encl_loader]: library allocation failed.\n");
    goto fail_close;
  }

  // Mmap the segment.
  library_plugin->size = (text_seg->p_memsz) + ((text_seg->p_memsz % text_seg->p_align) != 0) 
    * (text_seg->p_align - (text_seg->p_memsz % text_seg->p_align));
  library_plugin->plugin = mmap(NULL, library_plugin->size, PROT_READ|PROT_WRITE,
      MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE |MAP_FILE, -1, 0);
  if (library_plugin->plugin == MAP_FAILED) {
    fprintf(stderr, "[encl_loader]: mapping plugin failed.\n");
    goto fail_free;
  }

  // Copy the content
  if (lseek(fd, text_seg->p_offset, SEEK_SET) != text_seg->p_offset) {
    fprintf(stderr, "[encl_loader]: seeking text segment offset failed.\n");
    goto fail_unmap;
  }

  if(read(fd, library_plugin->plugin, text_seg->p_filesz) != text_seg->p_filesz) {
    fprintf(stderr, "[encl_loader]: reading text segment failed.\n");
    goto fail_unmap;
  }
  
  // Now mprotect.
  mprotect(library_plugin->plugin, library_plugin->size, PROT_READ|PROT_EXEC);

  // Now find the gate.
  Elf64_Sym* gate = find_symbol(fd, VMCALL_GATE_NAME, header, sections);
  if (gate == NULL) {
    fprintf(stderr, "[encl_loader]: no gate found.\n");
    goto fail_unmap;
  }

  library_plugin->vmcall_gate = (gate->st_value - text_seg->p_vaddr) + library_plugin->plugin; 
  
  free(gate);
  close(fd);
  // All good, return the plugin.
  return library_plugin; 

fail_unmap:
  munmap(library_plugin->plugin, library_plugin->size);
fail_free:
  free(library_plugin);
  library_plugin = NULL;
fail_close:
  close(fd);
fail:
  return NULL;
}


int load_enclave( const char* file,
                  tyche_encl_handle_t* handle,
                  struct tyche_encl_add_region_t* extras)
{
  // You need to initialize the library_plugin
  if (!library_plugin) {
    fprintf(stderr, "[encl_loader]: library_plugin is null.\n");
    goto fail;
  }
  load_encl_t enclave = {
    .driver_fd = 0,
    .elf_fd = 0,
    .sections = NULL,
    .segments = NULL,
    .stack_section = NULL,
  };
  if (handle == NULL) {
    goto fail;
  }
  // mmap the file in memory.
  enclave.elf_content = mmap_file(file, &(enclave.elf_fd));
  if (enclave.elf_content == NULL || enclave.elf_fd == -1) {
    fprintf(stderr, "[encl_loader]: mmap of enclave failed.\n");
    goto fail; 
  }

  // Parse the ELF file.
  if (parse_elf(&enclave) != 0) {
    fprintf(stderr, "[encl_loader]: unable to parse enclave.\n");
    goto fail_close;
  }

  // Create the enclave.
  if (create_enclave(&enclave, extras) != 0) {
    fprintf(stderr, "[encl_loader]: create enclave failure.\n");
    goto fail_free; 
  }
  
  // Setup the handle.
  *handle = enclave.handle; 
  return 0;
fail_free:
  free(enclave.sections);
  free(enclave.segments);
fail_close:
  close(enclave.elf_fd);
fail:
  return -1;
}