#ifndef __INCLUDE_TYCHE_CAPABILITIES_H__
#define __INCLUDE_TYCHE_CAPABILITIES_H__

#include "ecs.h"
#include "tyche_capabilities_types.h"

// ———————————————————————————————— Globals ————————————————————————————————— //

extern domain_t local_domain;

// —————————————————————————————————— API ——————————————————————————————————— //

/// Initialize the local domain.
/// This function enumerates the regions attributed to this domain and populates
/// the local_domain.
int init(capa_alloc_t allocator, capa_dealloc_t deallocator, capa_dbg_print_t print);

/// Creates a new domain.
/// Sets the result inside the provided handle.
int create_domain(unsigned long spawn, unsigned long comm);

#endif
