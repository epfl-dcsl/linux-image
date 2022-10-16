#include "tyche_vmcall.h"

int tyche_domain_create(struct enclave_t* encl)
{
  uint64_t result = -1;
  uint64_t handle = 0;
  if (!encl) {
    return -1;
  }

  asm volatile(
      "movq %2, %%rax\n\t"
      "vmcall\n\t"
      "movq %%rax, %1\n\t"
      "movq %%rcx, %0"
      : "=rm"(result), "=rm"(handle)
      : "r"((uint64_t)(TYCHE_DOMAIN_CREATE))
      : "rax", "rcx", "memory");
  if (result == 0) {
    encl->tyche_handle = handle;
  }
  return (int)result;
}
