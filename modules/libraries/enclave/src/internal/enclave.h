#ifndef __INTERNAL_ENCLAVE_H__
#define __INTERNAL_ENCLAVE_H__

#include <linux/types.h>

#define _IN_MODULE
#include "../../include/tyche_enclave.h"
#undef _IN_MODULE
#include "../../include/dll.h"

// ——————————————————————————— Type Declarations ———————————————————————————— //

// Forward declarations.
struct pa_region_t;
struct region_t;

/// Describes a contiguous region to allocate to an enclave.
struct region_t {
  /// Double linked list.
  dll_elem(struct region_t, list);

  /// Start address. Must be page aligned.
  uint64_t start;

  /// End address. Must be page aligned.
  uint64_t end;

  /// Protection flags (RWXU) for this region.
  uint64_t flags;

  /// Type of mapping: Confidential or Shared.
  enum tyche_encl_mapping_t tpe;

  /// List of corresponding physical pages.
  dll_list(struct pa_region_t, pas);
};

/// Describes physical memory regions.
struct pa_region_t {
  uint64_t start;
  uint64_t end;
  enum tyche_encl_mapping_t tpe;

  dll_elem(struct pa_region_t, list);
  dll_elem(struct pa_region_t, globals);
};

/// Describes an enclave.
struct enclave_t {
  /// The pid of the task that created this enclave.
  pid_t pid;

  /// Unique enclave identifier
  tyche_encl_handle_t handle;

  /// TODO add the physical value of cr3.
  /// TODO add the virtual value of cr3 (the root page).

  /// List of pages used by page tables.
  dll_list(struct pa_region_t, pts);

  /// Enclaves belong to a list.
  dll_elem(struct enclave_t, list);

  /// List of regions that belong to this enclave.
  dll_list(struct region_t, regions);

  /// List of physical regions associated with this enclave.
  dll_list(struct pa_region_t, all_pages);
};

// —————————————————————————————— Enclave API ——————————————————————————————— //
void enclave_init(void);
int add_enclave(tyche_encl_handle_t handle);
int add_region(struct tyche_encl_add_region_t* region);
int add_pa_to_region(struct region_t* region, struct pa_region_t** pa_region);
int commit_enclave(tyche_encl_handle_t handle);

#endif
