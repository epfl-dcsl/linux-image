#ifndef __INCLUDE_ENCL_LOADER_H__
#define __INCLUDE_ENCL_LOADER_H__

#include "elf64.h"
#include "encl_rt.h"
#include "tyche_enclave.h"

#define VMCALL_GATE_NAME "domain_gate_vmcall"

/// Type of entry functions in the enclave.
typedef void (*target_func_t)(void*);

/// Type of vmcall gate that should be provided by the libencl.
typedef void (*vmcall_gate_t)(tyche_encl_handle_t handle, target_func_t fn, void* args);

/// The configuration of an enclave library (encl.so).
typedef struct lib_encl_t {
  /// Where the dynload put the library.
  void* plugin;

  /// The size of the gate.
  size_t size;

  /// Gate implemented as vmcall.
  vmcall_gate_t vmcall_gate;

  /// TODO other gate mechanisms/interfaces provided by encl.so
} lib_encl_t;

// ————————————————————————————— Internal types ————————————————————————————— //

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

  /// The enclave entry point.
  Elf64_Sym* entry_point;

  /// Enclave handle.
  tyche_encl_handle_t handle;

  /// domain handle
  domain_id_t domain_handle;

  /// Where each segment is mapped.
  void** mappings;
  size_t* sizes;
} load_encl_t;

// ——————————————————————————————— Public API ——————————————————————————————— //

/// Initialize the enclave loader with a libencl.
/// The libencl will be mapped by default in all the enclaves.
const lib_encl_t* init_enclave_loader(const char* libencl);

/// Load the enclave defined by the file path, add the extras regions to it,
/// store the resulting handle in the pprovided pointer.
int load_enclave(const char* file, tyche_encl_handle_t* handle,
    domain_id_t* domain_handle, struct tyche_encl_add_region_t* extras);

// ————————————————————————————— Debugging API —————————————————————————————— //
void* mmap_file(const char* file, int* fd);
int parse_enclave(load_encl_t* enclave);
int map_enclave(load_encl_t* enclave);
int create_enclave(load_encl_t* enclave, struct tyche_encl_add_region_t* extras);
#endif
