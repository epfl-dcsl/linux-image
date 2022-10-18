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
