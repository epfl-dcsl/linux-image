#ifndef __INTERNAL_TYCHE_VMCALL_H__
#define __INTERNAL_TYCHE_VMCALL_H__

#include "enclave.h"

/// Copied from the tyche source code.
#define TYCHE_DOMAIN_GET_OWN_ID 0x100
#define TYCHE_DOMAIN_CREATE 0x101
#define TYCHE_DOMAIN_SEAL 0x102
#define TYCHE_REGION_SPLIT 0x200

/// Results from vmcalls are in rax (succes), rcx, rdx, rsi

int tyche_domain_create(struct enclave_t* encl);

#endif
