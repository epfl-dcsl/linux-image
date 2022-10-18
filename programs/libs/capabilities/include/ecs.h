#ifndef __INCLUDE_HARDWARE_CAPABILITIES_H__
#define __INCLUDE_HARDWARE_CAPABILITIES_H__

#include "types.h"
///! This file describes the API to talk with tyche when reading ECS.
/// It basically abstracts a very simple monitor API that reads 64 bits at
/// a given offset.
///
/// Conceptually, it is just an array of 64 bits entries where index 0 gives
/// the size of the populated array.

// ————————————————————————— Size-dependent offsets ————————————————————————— //
#define TYCHE_ECS_SZ_IDX ((index_t)0)

#define TYCHE_ECS_FIRST_ENTRY ((index_t)1)

// —————————————————————————————————— API ——————————————————————————————————— //

/// Read the size of the ECS.
int ecs_read_size(index_t* size);

/// Read the entry at idx from the ECS.
int ecs_read_entry(index_t idx, paddr_t* entry);

#endif
