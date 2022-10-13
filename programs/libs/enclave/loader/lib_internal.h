#ifndef __SRC_LIB_INTERNAL_H__
#define __SRC_LIB_INTERNAL_H__

#include "elf64.h"
#include "tyche_enclave.h"

///! Internal types for encl.so

/// encl_create_t holds all the information we need to create an enclave
/// from an mmaped and opened elf binary.
typedef struct load_encl_t {
  /// Enclave driver fd.
  int driver_fd;

  /// Binary ELF fd.
  int elf_fd;

  /// The file that was mmap-ed.
  void* elf_content;

  Elf64_Ehdr header;

  /// The ELF sections.
  Elf64_Shdr* sections;

  /// The ELF segments.
  Elf64_Phdr* segments;

  /// The stack segment.
  Elf64_Shdr* stack_section;

  /// Enclave handle.
  tyche_encl_handle_t handle;

  /// Where each segment is mapped.
  void** mappings;
  size_t* sizes;
} load_encl_t;

#endif
