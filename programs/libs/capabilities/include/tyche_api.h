#ifndef __INCLUDE_TYCHE_API_H__
#define __INCLUDE_TYCHE_API_H__

#include "types.h"

/// Copied from the tyche source code.
#define TYCHE_DOMAIN_GET_OWN_ID 0x100
#define TYCHE_DOMAIN_CREATE 0x101
#define TYCHE_DOMAIN_SEAL 0x102
#define TYCHE_ECS_READ 0x103
#define TYCHE_ECS_WRITE 0x104
#define TYCHE_READ_CAPA 0x105
#define TYCHE_REGION_SPLIT 0x200

/// Known offsets.
#define TYCHE_ECS_HDR_OFF ((index_t)0)
#define TYCHE_ECS_HDR_SZ ((index_t)3)
#define TYCHE_ECS_ENT_SZ ((index_t)3)

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

#endif
