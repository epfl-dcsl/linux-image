#ifndef __INCLUDE_ENCL_LOADER_H__
#define __INCLUDE_ENCL_LOADER_H__

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

/// Initialize the enclave loader with a libencl.
/// The libencl will be mapped by default in all the enclaves.
const lib_encl_t* init_enclave_loader(const char* libencl);

/// Load the enclave defined by the file path, add the extras regions to it,
/// store the resulting handle in the pprovided pointer.
int load_enclave(const char* file, tyche_encl_handle_t* handle, struct tyche_encl_add_region_t* extras);

#endif
