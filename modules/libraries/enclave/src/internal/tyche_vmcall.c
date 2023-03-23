#include <linux/kernel.h>
#include "tyche_vmcall.h"
#include "tyche_capabilities_types.h"

extern int tc_create_domain(domain_id_t* handle, unsigned long spawn, unsigned long comm);
extern int tc_seal_domain(domain_id_t dom, unsigned long core_map, paddr_t cr3, paddr_t entry, paddr_t stack);
extern int tc_grant_region(domain_id_t dom, paddr_t start, paddr_t end, memory_access_right_t access);
extern int tc_share_region(domain_id_t dom, paddr_t start, paddr_t end, memory_access_right_t access);
extern int tc_revoke_region(domain_id_t dom, paddr_t start, paddr_t end);


int tyche_domain_create(struct enclave_t* encl, usize spawn, usize comm)
{
  if (encl == NULL) {
    return -1;
  }
  return tc_create_domain(&(encl->tyche_handle), spawn, comm);
}

int tyche_share_grant(struct enclave_t* enclave, struct pa_region_t* region)
{
  if (enclave == NULL || region == NULL) {
    pr_err("[TE]: Error in split_grant, enclave or region is null.\n");
    return -1;
  } 
  if (region->tpe == CONFIDENTIAL) {
    return tc_grant_region(enclave->tyche_handle, region->start, region->end, region->flags);
  }
  return tc_share_region(enclave->tyche_handle, region->start, region->end, region->flags);
}

int tyche_seal_enclave(struct enclave_t* enclave)
{
  if (enclave == NULL) {
    pr_err("[TE]: Error enclave is null in tyche_seal_enclave.\n");
    return -1;
  } 
  //TODO allow to specify cores.
  return tc_seal_domain(enclave->tyche_handle, ALL_CORES_MAP, enclave->cr3, enclave->entry, enclave->stack);
}

int tyche_revoke_region(domain_id_t dom, paddr_t start, paddr_t end)
{
  return tc_revoke_region(dom, start, end);
}
