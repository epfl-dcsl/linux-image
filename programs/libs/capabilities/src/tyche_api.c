#include "tyche_api.h"

/// Simple generic vmcall implementation.
int tyche_call(vmcall_frame_t* frame)
{
  unsigned long result = 1;
  asm volatile(
    "movq %4, %%rax\n\t"
    "movq %5, %%rcx\n\t"
    "movq %6, %%rdx\n\n"
    "movq %7, %%rsi\n\t"
    "vmcall\n\t"
    "movq %%rax, %0\n\t"
    "movq %%rcx, %1\n\t"
    "movq %%rdx, %2\n\t"
    "movq %%rsi, %3\n\t"
    : "=rm" (result), "=rm" (frame->ret_1), "=rm" (frame->ret_2), "=rm" (frame->ret_3)
    : "rm" (frame->id), "rm" (frame->value_1), "rm" (frame->value_2), "rm" (frame->value_3)
    : "rax", "rcx", "rdx", "rsi", "memory");

  return (int)result;
} 

int tyche_get_domain_id(domain_id_t* domain)
{
  vmcall_frame_t frame;
  if (domain == NULL) {
    return -1;
  }
  frame.id= TYCHE_DOMAIN_GET_OWN_ID;
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  *domain = frame.ret_1;
  return 0;
}

int tyche_read_capa(paddr_t handle, paddr_t* start, paddr_t* end, capability_type_t* tpe)
{
  vmcall_frame_t frame;
  if (start == NULL || end == NULL || tpe == NULL) {
    return -1;
  } 

  frame.id = TYCHE_REGION_GET_INFO;
  frame.value_1 = handle;
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  *start = frame.ret_1;
  *end = frame.ret_2;
  *tpe = (capability_type_t) frame.ret_3;
  return 0;
}

int tyche_split_capa(paddr_t handle, paddr_t split_addr, paddr_t* new_handle)
{
  vmcall_frame_t frame;
  if (new_handle == NULL) {
    return -1;
  }
  frame.id= TYCHE_REGION_SPLIT;
  frame.value_1 = handle;
  frame.value_2 = split_addr;
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  *new_handle = frame.ret_1;
  return 0;
}

int tyche_grant_capa(domain_id_t target, paddr_t handle)
{
  //We should change that.
  vmcall_frame_t frame;
  frame.id = TYCHE_DOMAIN_GRANT_REGION;
  frame.value_1 = target;
  frame.value_2 = handle;
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  return 0;
}

int tyche_share_capa(domain_id_t target, paddr_t handle, capability_type_t tpe)
{
  vmcall_frame_t frame;
  frame.id = TYCHE_SHARE_CAPA;
  frame.value_1 = target;
  frame.value_2 = handle;
  frame.value_3 = tpe;
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  return 0;
}
