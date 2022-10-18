#ifndef __INCLUDE_HARDWARE_CAPABILITIES_H__
#define __INCLUDE_HARDWARE_CAPABILITIES_H__

#include "types.h"
///! This file describes the API to talk with tyche when reading ECS.
/// It basically abstracts a very simple monitor API that reads 64 bits at
/// a given offset.
///
/// Conceptually, it is just an array implementation of a linked list.
/// The first three 64 bits are the header entry.
/// Each element is then three 64 bits long too, containing the handle,
/// a prev and next element.

// ———————————————————————————————— Globals ————————————————————————————————— //
//TODO eventually make that a queriable thing from tyche.
#define MAX_ECS_ENTRIES ((index_t)1000)

// ————————————————————————————————— Types —————————————————————————————————— //
/// An ecs_entry, it has:
/// 1) the current handle.
/// 2) the index of the previous entry.
/// 3) the index of the next entry.
/// Invalid indices are set to 0 as only the header can be zero.
typedef struct ecs_entry_t {
  paddr_t handle;
  index_t prev;
  index_t next;
} ecs_entry_t;

/// The header is the first entry of the ECS.
typedef struct ecs_header_t {
  // Size of the linked list.
  index_t size;

  // Head of the linked list.
  index_t head;

  // Tail of the linked list.
  index_t tail;
} ecs_header_t;

// ————————————————————————— Size-dependent offsets ————————————————————————— //
#define TYCHE_ECS_STEP (sizeof(ecs_entry_t) / sizeof(index_t))
#define TYCHE_ECS_HDR_OFF ((index_t)0)
#define TYCHE_ECS_HDR_SZ ((index_t)(sizeof(ecs_header_t) / sizeof(index_t)))
#define TYCHE_ECS_ENT_SZ ((index_t)(TYCHE_ECS_STEP))

#define TYCHE_ECS_FIRST_IDX (TYCHE_ECS_HDR_SZ)
#define TYCHE_ECS_LAST_IDX (TYCHE_ECS_FIRST_IDX + MAX_ECS_ENTRIES * TYCHE_ECS_STEP)

// —————————————————————————————————— API ——————————————————————————————————— //

/// Read the header.
int read_header(ecs_header_t* header);

/// Write the header.
int write_header(ecs_header_t* header);

/// Read an entry.
int read_entry(index_t idx, ecs_entry_t* entry);

/// Write an entry.
int write_entry(index_t idx, ecs_entry_t* entry);

#endif
