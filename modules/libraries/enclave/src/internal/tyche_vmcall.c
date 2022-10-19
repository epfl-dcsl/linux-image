#include "tyche_vmcall.h"
#include "tyche_capabilities_types.h"

extern int tc_create_domain(domain_id_t* handle);

int tyche_domain_create(struct enclave_t* encl)
{
  if (encl == NULL) {
    return -1;
  }
  return tc_create_domain(&(encl->handle));
}

int tyche_split_grant(struct enclave_t* enclave, struct pa_region_t* region)
{
  //TODO
  return 0;
}
