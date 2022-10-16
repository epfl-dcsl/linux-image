#ifndef __INTERNAL_TYCHE_VMCALL_H__
#define __INTERNAL_TYCHE_VMCALL_H__

#include "enclave.h"

/// Copied from the tyche source code.
#define TYCHE_DOMAIN_CREATE 0x101

/// Results from vmcalls are in rax (succes), rcx, rdx, rsi

int tyche_domain_create(struct enclave_t* encl);

#endif
