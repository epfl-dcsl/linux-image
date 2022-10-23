#include <linux/kernel.h>
#include "tyche_vmcall.h"
#include "tyche_capabilities_types.h"

extern int tc_create_domain(domain_id_t* handle);
extern int tc_transfer_capability(domain_id_t dom, paddr_t start, paddr_t end, capability_type_t tpe, paddr_t* new_handle);

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
