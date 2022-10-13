#ifndef __INCLUDE_ENCL_H__
#define __INCLUDE_ENCL_H__

#include "tyche_enclave.h"

/// Load the enclave defined by the file path.
int load_enclave(const char* file, tyche_encl_handle_t* handle);

#endif
