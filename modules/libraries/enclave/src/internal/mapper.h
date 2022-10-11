#ifndef __INTERNAL_MAPPER_H__
#define __INTERNAL_MAPPER_H__

#include "enclave.h"
#include "x86_64_pt.h"

typedef struct map_info_t {
  // The enclave's root cr3.
  entry_t cr3;
  // Intermediary flags to be used in the page tables.
  entry_t intermed_flags;
  // The entry we want for this flag. (Not sure it is needed here);
  entry_t entry_flags;

  pt_profile_t* profile;
  struct enclave_t* enclave;
} map_info_t;

int build_enclave_cr3(struct enclave_t* encl);

#endif
