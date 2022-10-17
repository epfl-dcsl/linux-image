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
      "movq %%rax, %0\n\t"
      "movq %%rcx, %1"
      : "=rm"(result), "=rm"(handle)
      : "r"((uint64_t)(TYCHE_DOMAIN_CREATE))
      : "rax", "rcx", "memory");
  if (result == 0) {
    encl->tyche_handle = handle;
  }
  return (int)result;
}

int tyche_split_grant(struct enclave_t* enclave, struct pa_region_t* region)
{
  //TODO
  return 0;
}
