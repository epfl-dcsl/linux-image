#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

// elf64 library includes.
#include "elf64.h"
// local includes.
#include "encl.h"
#include "lib_internal.h"

const char* STACK_SECTION_NAME = ".encl_stack";
const char* ENCL_DRIVER = "/dev/tyche_enclave"; 

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
  Elf64_Ehdr header; 
  read_elf64_header(encl->elf_fd, &(encl->header));
  
  // Segments to map.
  read_elf64_segments(encl->elf_fd, encl->header, &(encl->segments));
  
  // Sections: find the stack.
  // Later we can use that part to divide kernel vs. user code too.
  read_elf64_sections(encl->elf_fd, encl->header, &(encl->sections));
  char *sh_names = read_section64(encl->elf_fd, encl->sections[encl->header.e_shstrndx]); 
  for (int i = 0; i < header.e_shnum; i++) {
    if (strcmp(sh_names + encl->sections[i].sh_name, STACK_SECTION_NAME) == 0) {
      encl->stack_section = &(encl->sections[i]);
      break;
    } 
  }
  free(sh_names);
  sh_names = NULL;
  // Failure? 
  if(encl->stack_section == NULL) {
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
  uint64_t result = 0;
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

static int create_enclave(load_encl_t* enclave)
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
      .end = segment.p_vaddr + segment.p_memsz,
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

int load_enclave(const char* file, tyche_encl_handle_t* handle)
{
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
    goto fail; 
  }

  // Parse the ELF file.
  if (parse_elf(&enclave) != 0) {
    goto fail_close;
  }

  // Create the enclave.
  if (create_enclave(&enclave) != 0) {
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
