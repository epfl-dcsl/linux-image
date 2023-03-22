#include "tyche_api.h"

/// Simple generic vmcall implementation.
int tyche_call(vmcall_frame_t* frame)
{
  unsigned long result = FAILURE;
  asm volatile(
    "movq %7, %%rax\n\t"
    "movq %8, %%rdi\n\t"
    "movq %9, %%rsi\n\n"
    "movq %10, %%rdx\n\t"
    "movq %11, %%rcx\n\t"
    "movq %12, %%r8\n\t"
    "movq %13, %%r9\n\t"
    "movq %14, %%r10\n\t"
    "vmcall\n\t"
    "movq %%rax, %0\n\t"
    "movq %%rdi, %1\n\t"
    "movq %%rsi, %2\n\t"
    "movq %%rdx, %3\n\t"
    "movq %%rcx, %4\n\t"
    "movq %%r8,  %5\n\t"
    "movq %%r9,  %6\n\t"
    : "=rm" (result), "=rm" (frame->value_1), "=rm" (frame->value_2), "=rm" (frame->value_3), "=rm" (frame->value_4), "=rm" (frame->value_5), "=rm" (frame->value_6)
    : "rm" (frame->vmcall), "rm" (frame->arg_1), "rm" (frame->arg_2), "rm" (frame->arg_3), "rm" (frame->arg_4), "rm" (frame->arg_5), "rm" (frame->arg_6), "rm" (frame->arg_7) 
    : "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "memory");

  return (int)result;
} 

int tyche_create_domain(
    capa_index_t* self,
    capa_index_t* child,
    capa_index_t* revocation,
    unsigned long spawn,
    unsigned long comm)
{
  vmcall_frame_t frame;
  if (self == NULL || child == NULL || revocation == NULL) {
    goto fail;
  }
  frame.vmcall = TYCHE_CREATE_DOMAIN;
  frame.arg_1 = spawn;
  frame.arg_2 = comm;
  if (tyche_call(&frame) != SUCCESS) {
    goto fail;
  }
  *self = frame.value_1;
  *child = frame.value_2;
  *revocation = frame.value_3;
  return SUCCESS;
fail:
  return FAILURE;
}

int tyche_seal(
    capa_index_t* transition, 
    capa_index_t unsealed,
    unsigned long core_map,
    unsigned long cr3,
    unsigned long rip, 
    unsigned long rsp)
{
  vmcall_frame_t frame = {
    .vmcall = TYCHE_SEAL_DOMAIN,
    .arg_1 = unsealed,
    .arg_2 = core_map,
    .arg_3 = cr3, 
    .arg_4 = rip,
    .arg_5 = rsp,
  };
  if (transition == NULL) {
    goto failure;
  }

  if (tyche_call(&frame) != SUCCESS) {
    goto failure;
  }
  *transition = frame.value_1;
  return SUCCESS;
failure:
  return FAILURE;
}

int tyche_duplicate(
    capa_index_t* left,
    capa_index_t* right,
    capa_index_t capa,
    unsigned long a1_1,
    unsigned long a1_2,
    unsigned long a1_3,
    unsigned long a2_1,
    unsigned long a2_2,
    unsigned long a2_3)
{
  vmcall_frame_t frame = {
    TYCHE_DUPLICATE,
    a1_1,
    a1_2,
    a1_3,
    a2_1,
    a2_2,
    a2_3,
  };
  if (left == NULL || right == NULL) {
    goto failure;
  }
  if (tyche_call(&frame) != SUCCESS) {
    goto failure;
  } 
  *left = frame.value_1;
  *right = frame.value_2;
  return SUCCESS;
failure:
  return FAILURE;
}

int tyche_grant(
    capa_index_t dest,
    capa_index_t capa,
    unsigned long a1,
    unsigned long a2,
    unsigned long a3)
{
  vmcall_frame_t frame = {
    .vmcall = TYCHE_GRANT,
    .arg_1 = dest,
    .arg_2 = capa,
    .arg_3 = a1,
    .arg_4 = a2,
    .arg_5 = a3,
  };
  if (tyche_call(&frame) != SUCCESS) {
    goto failure;
  }
  // Check that the revocation handle is the original one.
  if (frame.value_1 != capa) {
    goto failure;
  }
  return SUCCESS;
failure:
  return FAILURE;
}

int tyche_share(
    capa_index_t* left,
    capa_index_t dest,
    capa_index_t capa,
    unsigned long a1,
    unsigned long a2,
    unsigned long a3)
{
  vmcall_frame_t frame = {
    .vmcall = TYCHE_SHARE,
    .arg_1 = dest,
    .arg_2 = capa,
    .arg_3 = a1,
    .arg_4 = a2,
    .arg_5 = a3
  };
  if (left == NULL || tyche_call(&frame) != SUCCESS) {
    goto failure;
  }
  *left = frame.value_1; 
  return SUCCESS;
failure:
  return FAILURE;
}

int tyche_revoke(capa_index_t id)
{
  vmcall_frame_t frame = {
    .vmcall = TYCHE_REVOKE,
    .arg_1 = id,
  };
  if (tyche_call(&frame) != SUCCESS) {
    goto failure;
  }
  return SUCCESS;
failure:
  return FAILURE;
}
