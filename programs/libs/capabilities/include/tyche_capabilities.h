#ifndef __INCLUDE_TYCHE_CAPABILITIES_H__
#define __INCLUDE_TYCHE_CAPABILITIES_H__

#include "dll.h"

/// Internal definition of our types so we can move to 32 bits.
typedef unsigned long paddr_t;

/// Internal definition of domain id.
typedef unsigned long domain_id_t;

/// Valid types for a capability.
typedef enum capability_type_t {
  Shared = 0,
  Confidential = 1,
  Other = 3,
} capability_type_t;

/// Capability that confers access to a memory region.
typedef struct capability_t {
  size_t id;
  paddr_t start;
  paddr_t end;
  capability_type_t tpe;

  // This structure can be put in a double-linked list
  dll_elem(struct capability_t, list);
} capability_t;

typedef void* (*capa_alloc_t)(size_t size);

/// Represents the current domain's metadata.
typedef struct domain_t {
  // The id of the current domain.
  domain_id_t id;
  // The list of capabilities for this domain.
  dll_list(struct capability_type_t, capabilities);
  // The allocator to use whenever we need a new structure.
  capa_alloc_t allocator;
} domain_t;

// ———————————————————————————————— Globals ————————————————————————————————— //
extern domain_t local_domain;

// —————————————————————————————————— API ——————————————————————————————————— //

/// Initialize the local domain.
/// This function enumerates the regions attributed to this domain and populates
/// the local_domain.
int init_domain(capa_alloc_t allocator);

/// Transfer the ownership of a given region to another domain.
/// This requires to check, for example, whenever tpe is confidential,
/// to ensure that the corresponding parent handle is solely owned by the local
/// domain.
/// @warn: this function implements both grant and share.
int transfer_capa(domain_id_t dom, paddr_t start, paddr_t end, capability_type_t tpe);

/// Merges the referenced capability defined by start/end into the local domain.
int merge_capa(domain_id_t owner, paddr_t start, paddr_t end, capability_type_t tpe);

#endif
