#ifndef __INTERNAL_TYCHE_VMCALL_H__
#define __INTERNAL_TYCHE_VMCALL_H__

#include "enclave.h"

int tyche_domain_create(struct enclave_t* encl);
int tyche_split_grant(struct enclave_t* enclave, struct pa_region_t* region);
#endif
