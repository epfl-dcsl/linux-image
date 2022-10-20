#ifndef __INCLUDE_TYCHE_CAPABILITIES_TYPES_H__
#define __INCLUDE_TYCHE_CAPABILITIES_TYPES_H__

#ifndef NULL
#define NULL ((void*)0)
#endif

/// Internal definition of our types so we can move to 32 bits.
typedef long long unsigned int paddr_t;

/// Internal definition of domain id.
typedef unsigned long domain_id_t;

/// Internal definition of index.
typedef unsigned long capa_index_t;

/// Valid types for a capability.
typedef enum capability_type_t {
  PtEntry = 0,
  Confidential = 1,
  Shared,
  Other = 3,
} capability_type_t;

#endif
