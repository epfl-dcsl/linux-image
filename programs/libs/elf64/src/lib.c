#include "common.h"
#include "elf64.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


void read_elf64_header(int fd, Elf64_Ehdr *eh)
{
  TEST(eh != NULL); 
  TEST(lseek(fd, (off_t)0, SEEK_SET) == (off_t)0);
  TEST(read(fd, (void*)eh, sizeof(Elf64_Ehdr)) == sizeof(Elf64_Ehdr)); 
  TEST(strncmp((char*)eh->e_ident, ELFMAG, SELFMAG) == 0);
}

size_t read_elf64_sections(int fd, Elf64_Ehdr eh, Elf64_Shdr** sections)
{
  TEST(sections!=NULL);
  TEST(eh.e_shnum > 0);
  *sections = calloc(sizeof(Elf64_Shdr), eh.e_shnum);
  TEST(*sections != NULL);
  TEST(lseek(fd, eh.e_shoff, SEEK_SET) == eh.e_shoff);
  for (int i = 0; i < eh.e_shnum; i++) {
    TEST(read(fd, (void*)(&(*sections[i])), sizeof(Elf64_Shdr)) == sizeof(Elf64_Shdr));
  }
  return eh.e_shnum;
}

size_t read_elf64_segments(int fd, Elf64_Ehdr eh, Elf64_Phdr** segments)
{
  TEST(segments != NULL);
  TEST(eh.e_phnum > 0);
  *segments = calloc(sizeof(Elf64_Phdr), eh.e_phnum);
  TEST(*segments != NULL);
  TEST(lseek(fd, eh.e_phoff, SEEK_SET) == eh.e_phoff);
  TEST(sizeof(Elf64_Phdr) == eh.e_phentsize);
  for (int i = 0; i < eh.e_phnum; i++) {
    TEST(read(fd, (void*)(&(*segments[i])), sizeof(Elf64_Phdr)) == sizeof(Elf64_Phdr)); 
  }
  return eh.e_phnum;
}

void* read_section64(int fd, Elf64_Shdr sh)
{
  void* result = malloc(sh.sh_size);
  TEST(result != NULL);
  TEST(lseek(fd, sh.sh_offset, SEEK_SET) == sh.sh_offset);
  TEST(read(fd, result, sh.sh_size) == sh.sh_size);
  return result;
}

Elf64_Sym* find_symbol_in_section(int fd, char* symbol, Elf64_Ehdr eh, Elf64_Shdr sections[], int idx)
{
  Elf64_Sym* result = NULL;
  Elf64_Sym* sym_tbl = (Elf64_Sym*)read_section64(fd, sections[idx]); 
  TEST(sym_tbl != NULL);
  Elf64_Word str_tbl_ndx = sections[idx].sh_link;
  TEST(str_tbl_ndx < eh.e_shnum);
  char* str_tbl = (char*)read_section64(fd, sections[str_tbl_ndx]);
  TEST((sections[idx].sh_size % sizeof(Elf64_Sym)) == 0);
  size_t symbol_count = (sections[idx].sh_size / sizeof(Elf64_Sym));
  for (int i = 0; i < symbol_count; i++) {
    char* entry = str_tbl + sym_tbl[i].st_name;
    if (strcmp(symbol, entry)) {
      result = malloc(sizeof(Elf64_Sym));
      TEST(result != NULL);
      memcpy(result, &sym_tbl[i], sizeof(Elf64_Sym));
      break;
    }
  }
  // Cleanup
  free(sym_tbl);
  free(str_tbl);
  return result;
}

Elf64_Sym* find_symbol(int fd, char* symbol, Elf64_Ehdr eh, Elf64_Shdr sections[])
{
  Elf64_Sym* result = NULL;
  TEST(symbol != NULL);
  for (int i = 0; i < eh.e_shnum; i++) {
    if ((sections[i].sh_type == SHT_DYNSYM) || (sections[i].sh_type = SHT_SYMTAB)) {
      result = find_symbol_in_section(fd, symbol, eh, sections, i);
      if (result != NULL) {
        return result;
      }
    }
  }
  TEST(result = NULL);
  return NULL;
}
