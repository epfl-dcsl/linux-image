#ifndef __INCLUDE_TYCHE_API_H__
#define __INCLUDE_TYCHE_API_H__

#include "types.h"

/// Copied from the tyche source code.
#define TYCHE_DOMAIN_GET_OWN_ID 0x100
#define TYCHE_DOMAIN_CREATE 0x101
#define TYCHE_DOMAIN_SEAL 0x102
#define TYCHE_ECS_READ 0x103
#define TYCHE_READ_CAPA 0x105
#define TYCHE_REGION_SPLIT 0x200
#define TYCHE_GRANT_CAPA 0x201
#define TYCHE_SHARE_CAPA 0x203

//TODO we need free spots too.

#define TYCHE_CAPA_NULL ((index_t)0)

/// A type to pass arguments and receive when calling tyche.
typedef struct vmcall_frame_t {
  // Vmcall id.
  unsigned long id;

  // Arguments.
  unsigned long value_1;
  unsigned long value_2;
  unsigned long value_3;

  // Results.
  unsigned long ret_1;
  unsigned long ret_2;
  unsigned long ret_3;
} vmcall_frame_t;

// —————————————————————————————————— API ——————————————————————————————————— //
int tyche_call(vmcall_frame_t* frame);
int tyche_get_domain_id(domain_id_t* domain);
int tyche_read_capa(paddr_t handle, paddr_t* start, paddr_t* end, capability_type_t* tpe);
int tyche_split_capa(paddr_t handle, paddr_t split_addr, paddr_t* new_handle);
int tyche_grant_capa(domain_id_t target, paddr_t handle, capability_type_t tpe);
int tyche_share_capa(domain_id_t target, paddr_t handle, capability_type_t tpe);

#endif
