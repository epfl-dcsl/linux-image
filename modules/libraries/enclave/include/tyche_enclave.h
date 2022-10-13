#ifndef __INCLUDE_TYCHE_ENCLAVE_H__
#define __INCLUDE_TYCHE_ENCLAVE_H__

#ifdef _IN_MODULE
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <stdint.h>
#include <sys/ioctl.h>
#endif

// ———————————————————— Constants Defined in the Module ————————————————————— //
#define TE_READ ((uint64_t)1 << 0)
#define TE_WRITE ((uint64_t)1 << 1)
#define TE_EXEC ((uint64_t)1 << 2)
#define TE_USER ((uint64_t)1 << 3)

// —————————————————————— Types Exposed by the Library —————————————————————— //
typedef uint64_t tyche_encl_handle_t;

/// Message type to create a new enclave.
struct tyche_encl_create_t {
  tyche_encl_handle_t handle;
};

enum tyche_encl_mapping_t {
  Confidential,
  Shared,
  PtEntry,
};

/// Message type to add a new region.
struct tyche_encl_add_region_t {
  /// Unique enclave reference capability.
  tyche_encl_handle_t handle;

  /// Start address. Must be page aligned.
  uint64_t start;

  /// End address. Must be page aligned.
  uint64_t end;

  /// Source for the content of the region.
  uint64_t src;

  /// Protection flags (RWXU) for this region.
  uint64_t flags;

  /// Type of mapping: Confidential or Shared.
  enum tyche_encl_mapping_t tpe;
};

// ——————————————————————————— Tyche Enclave IOCTL API —————————————————————— //
#define TYCHE_ENCLAVE_DBG _IO('a', 'a')
#define TYCHE_ENCLAVE_CREATE _IOR('a', 'b', struct tyche_encl_create_t*)
#define TYCHE_ENCLAVE_ADD_REGION _IOW('a', 'c', struct tyche_encl_add_region_t*)
#define TYCHE_ENCLAVE_COMMIT _IOW('a', 'd', tyche_encl_handle_t)

#endif
