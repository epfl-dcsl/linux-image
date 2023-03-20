#ifndef __INCLUDE_TYCHE_API_H__
#define __INCLUDE_TYCHE_API_H__

#include "tyche_capabilities_types.h"

/// Copied from the tyche source code
typedef enum tyche_monitor_call_t {
  TYCHE_CREATE_DOMAIN = 1,
  TYCHE_SEAL_DOMAIN = 2,
  TYCHE_SHARE = 3,
  TYCHE_GRANT = 4,
  TYCHE_GIVE = 5,
  TYCHE_REVOKE = 6,
  TYCHE_DUPLICATE = 7,
  TYCHE_ENUMERATE = 8,
  TYCHE_SWITCH = 9,
  TYCHE_EXIT = 10,
} tyche_monitor_call_t;

#define TYCHE_CAPA_NULL ((capa_index_t)0)

/// Defined in capabilities/src/domain.rs
#define CAPAS_PER_DOMAIN ((capa_index_t)100)

/// A type to pass arguments and receive when calling tyche.
typedef struct vmcall_frame_t {
  // Vmcall id.
  unsigned long vmcall;

  // Arguments.
  unsigned long arg_1;
  unsigned long arg_2;
  unsigned long arg_3;
  unsigned long arg_4;
  unsigned long arg_5;
  unsigned long arg_6;
  unsigned long arg_7;

  // Results.
  unsigned long value_1;
  unsigned long value_2;
  unsigned long value_3;
  unsigned long value_4;
  unsigned long value_5;
  unsigned long value_6;
} vmcall_frame_t;

// —————————————————————————————————— API ——————————————————————————————————— //
int tyche_call(vmcall_frame_t* frame);
int tyche_create_domain(
    capa_index_t* self,
    capa_index_t* child,
    capa_index_t* revocation,
    unsigned long spawn,
    unsigned long comm);

int tyche_duplicate(
    capa_index_t* left,
    capa_index_t* right,
    capa_index_t capa,
    unsigned long a1_1,
    unsigned long a1_2,
    unsigned long a1_3,
    unsigned long a2_1,
    unsigned long a2_2,
    unsigned long a2_3);

int tyche_grant(
    capa_index_t dest,
    capa_index_t capa,
    unsigned long a1,
    unsigned long a2,
    unsigned long a3);

#endif
