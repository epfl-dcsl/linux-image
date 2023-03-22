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
/// Sets the result inside the provided id handle.
int create_domain(domain_id_t* id, unsigned long spawn, unsigned long comm);

/// Seal the domain.
/// This function first creates a channel for the child domain and then seals it.
int seal_domain(
    domain_id_t id,
    unsigned long core_map,
    unsigned long cr3,
    unsigned long rip,
    unsigned long rsp);

/// Duplicate capability.
int duplicate_capa(
    capability_t** left,
    capability_t** right,
    capability_t* capa,
    unsigned long a1_1,
    unsigned long a1_2,
    unsigned long a1_3,
    unsigned long a2_1,
    unsigned long a2_2,
    unsigned long a2_3);

/// Grant a memory region.
/// Finds the correct capability and grants the region to the target domain.
int grant_region(domain_id_t id, paddr_t start, paddr_t end, memory_access_right_t access);

/// Share a memory region.
/// Finds the correct capability and shares the region with the target domain.
int share_region(domain_id_t id, paddr_t start, paddr_t end, memory_access_right_t access);

/// Revoke the memory region.
/// Start and end must match existing bounds on a capability.
int revoke_region(domain_id_t id, paddr_t start, paddr_t end);

/// Switch to the target domain.
/// Fails if all transition handles are used.
int switch_domain(domain_id_t id);

#endif
