#include <linux/kernel.h>
#include "tyche_vmcall.h"
#include "tyche_capabilities_types.h"

extern int tc_create_domain(domain_id_t* handle);
extern int tc_transfer_capability(domain_id_t dom, paddr_t start, paddr_t end, capability_type_t tpe, paddr_t* new_handle);
extern int tc_seal_domain(domain_id_t dom, paddr_t cr3, paddr_t stack);

int tyche_domain_create(struct enclave_t* encl)
{
  if (encl == NULL) {
    return -1;
  }
  return tc_create_domain(&(encl->tyche_handle));
}

int tyche_split_grant(struct enclave_t* enclave, struct pa_region_t* region)
{
  if (enclave == NULL || region == NULL) {
    pr_err("[TE]: Error in split_grant, enclave or region is null.\n");
    return -1;
  } 
  return tc_transfer_capability(enclave->tyche_handle, region->start,
      region->end, region->tpe, &(region->handle));
}

int tyche_seal_enclave(struct enclave_t* enclave)
{
  if (enclave == NULL) {
    pr_err("[TE]: Error enclave is null in tyche_seal_enclave.\n");
    return -1;
  } 
  return tc_seal_domain(enclave->tyche_handle, enclave->cr3, enclave->stack);
}
