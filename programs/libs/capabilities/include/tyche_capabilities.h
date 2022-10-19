#ifndef __INCLUDE_TYCHE_CAPABILITIES_H__
#define __INCLUDE_TYCHE_CAPABILITIES_H__

#include "dll.h"
#include "ecs.h"
#include "tyche_capabilities_types.h"

#define ALIGNMENT (0x1000)

/// Capability that confers access to a memory region.
typedef struct capability_t {
  capa_index_t index;
  paddr_t handle;
  paddr_t start;
  paddr_t end;
  capability_type_t tpe;

  // This structure can be put in a double-linked list
  dll_elem(struct capability_t, list);
} capability_t;

typedef void* (*capa_alloc_t)(unsigned long size);
typedef void (*capa_dealloc_t)(void* ptr);

/// Represents the current domain's metadata.
typedef struct domain_t {
  // The id of the current domain.
  domain_id_t id;

  // The allocator to use whenever we need a new structure.
  capa_alloc_t alloc;
  capa_dealloc_t dealloc;

  // The list of used capabilities for this domain.
  dll_list(struct capability_t, capabilities);
} domain_t;

// —————————————————————————————————— API ——————————————————————————————————— //

/// Initialize the local domain.
/// This function enumerates the regions attributed to this domain and populates
/// the local_domain.
int init_domain(capa_alloc_t allocator, capa_dealloc_t deallocator);

/// Creates a new domain.
/// Sets the result inside the provided handle.
int create_domain(domain_id_t* handle);

/// Splits the capability capa at address split_addr to create and return the split one
/// with tpe type.
/// The return value is taken from local_domain.frees.
capability_t* split_capa(capability_t* capa, paddr_t split_addr);

/// Transfer the ownership of a given region to another domain.
/// This requires to check, for example, whenever tpe is confidential,
/// to ensure that the corresponding parent handle is solely owned by the local
/// domain.
/// @warn: this function implements both grant and share.
int transfer_capa(domain_id_t dom, paddr_t start, paddr_t end, capability_type_t tpe);

/// Merges the referenced capability defined by start/end into the local domain.
int merge_capa(domain_id_t owner, paddr_t start, paddr_t end, capability_type_t tpe);

#endif
