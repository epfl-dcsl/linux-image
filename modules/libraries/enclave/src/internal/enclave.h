#ifndef __INTERNAL_ENCLAVE_H__
#define __INTERNAL_ENCLAVE_H__

#include <linux/types.h>

#define _IN_MODULE
#include "../../include/tyche_enclave.h"
#undef _IN_MODULE
#include "../../include/dll.h"

// ——————————————————————————— Type Declarations ———————————————————————————— //

// Forward declarations.
struct page_elem_t;
struct region_elem_t;

/// Describes a contiguous region to allocate to an enclave.
struct region_elem_t {
  /// Double linked list.
  dll_elem(struct region_elem_t, list);

  /// Start address. Must be page aligned.
  uint64_t start;

  /// End address. Must be page aligned.
  uint64_t end;

  /// Protection flags (RWXU) for this region.
  uint64_t flags;

  /// Type of mapping: Confidential or Shared.
  enum tyche_encl_mapping_t tpe;

  /// List of corresponding physical pages.
  dll_list(struct page_elem_t, pas);
};

/// Describes physical memory regions.
struct page_elem_t {
  dll_elem(struct pt_elem_t, list);
  uint64_t start;
  uint64_t end;
};

/// Describes an enclave.
struct enclave_t {
  /// Unique enclave identifier
  tyche_encl_handle_t handle;

  /// List of regions that belong to this enclave.
  dll_list(struct region_elem_t, regions);

  /// List of pages used by page tables.
  dll_list(struct page_elem_t, pts);

  /// Enclaves belong to a list.
  dll_elem(struct enclave_t, list);
};

// —————————————————————————————— Enclave API ——————————————————————————————— //
void enclave_init(void);
int add_enclave(tyche_encl_handle_t handle);

#endif
