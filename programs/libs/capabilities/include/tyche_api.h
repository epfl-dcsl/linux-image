#ifndef __INCLUDE_TYCHE_API_H__
#define __INCLUDE_TYCHE_API_H__

#include "tyche_capabilities_types.h"

/// Copied from the tyche source code
#define TYCHE_CREATE_DOMAIN 1
#define TYCHE_SEAL_DOMAIN 2
#define TYCHE_SHARE 3
#define TYCHE_GRANT 4
#define TYCHE_GIVE 5
#define TYCHE_REVOKE 6
#define TYCHE_ENUMERATE 7
#define TYCHE_SWITCH 8
#define TYCHE_EXIT 9

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

#endif
