#ifndef __INTERNAL_MAPPER_H__
#define __INTERNAL_MAPPER_H__

#include "enclave.h"
#include "x86_64_pt.h"

typedef struct map_info_t {
  pt_profile_t* profile;
  struct enclave_t* enclave;
} map_info_t;

int build_enclave_cr3(struct enclave_t* encl);

#endif
