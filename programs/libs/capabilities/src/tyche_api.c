#include "tyche_api.h"

/// Simple generic vmcall implementation.
int tyche_call(vmcall_frame_t* frame)
{
  unsigned long result = 1;
  asm volatile(
    "movq %7, %%rax\n\t"
    "movq %8, %%rdi\n\t"
    "movq %9, %%rsi\n\n"
    "movq %10, %%rdx\n\t"
    "movq %11, %%rcx\n\t"
    "movq %12, %%r8\n\t"
    "vmcall\n\t"
    "movq %%rax, %0\n\t"
    "movq %%rdi, %1\n\t"
    "movq %%rsi, %2\n\t"
    "movq %%rdx, %3\n\t"
    "movq %%rcx, %4\n\t"
    "movq %%r8,  %5\n\t"
    "movq %%r9,  %6\n\t"
    : "=rm" (result), "=rm" (frame->value_1), "=rm" (frame->value_2), "=rm" (frame->value_3), "=rm" (frame->value_4), "=rm" (frame->value_5), "=rm" (frame->value_6)
    : "rm" (frame->vmcall), "rm" (frame->arg_1), "rm" (frame->arg_2), "rm" (frame->arg_3), "rm" (frame->arg_4), "rm" (frame->arg_5) 
    : "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "memory");

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
  if (tyche_call(&frame) != 0) {
    goto fail;
  }
  *self = frame.value_1;
  *child = frame.value_2;
  *revocation = frame.value_3;
  return 0;
fail:
  return -1;
}
