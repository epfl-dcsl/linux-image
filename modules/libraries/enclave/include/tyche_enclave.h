#ifndef _TYCHE_ENCLAVE
#define _TYCHE_ENCLAVE

#ifdef _IN_MODULE
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <stdint.h>
#include <sys/ioctl.h>
#endif

// ———————————————————— Constants Defined in the Module ————————————————————— //
const uint64_t TE_READ = 1 << 0;
const uint64_t TE_WRITE = 1 << 1;
const uint64_t TE_EXEC = 1 << 2;
const uint64_t TE_USER = 1 << 3;

// —————————————————————— Types Exposed by the Library —————————————————————— //
typedef uint64_t tyche_encl_handle_t;

/// Message type to create a new enclave.
struct tyche_encl_create_t {
  tyche_encl_handle_t handle;
};

enum tyche_encl_mapping_t {
  Confidential,
  Shared,
};

/// Message type to add a new region.
struct tyche_encl_add_region_t {
  /// Unique enclave reference capability.
  tyche_encl_handle_t handle;

  /// Start address. Must be page aligned.
  uint64_t start;

  /// End address. Must be page aligned.
  uint64_t end;

  /// Protection flags (RWXU) for this region.
  uint64_t flags;

  /// Type of mapping: Confidential or Shared.
  enum tyche_encl_mapping_t tpe;
};

// ——————————————————————————— Tyche Enclave IOCTL API —————————————————————— //
#define TYCHE_ENCLAVE_DBG _IO('a', 'a')
#define TYCHE_ENCLAVE_CREATE _IOR('a', 'a', struct tyche_encl_create_t*)
#define TYCHE_ENCLAVE_ADD_REGION _IOW('a', 'b', struct tyche_encl_add_region_t*)

#endif
