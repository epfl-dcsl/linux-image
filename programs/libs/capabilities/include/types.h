#ifndef __INCLUDE_CAPA_TYPES_H__
#define __INCLUDE_CAPA_TYPES_H__

#ifndef NULL
#define NULL ((void*)0)
#endif

/// Internal definition of our types so we can move to 32 bits.
typedef unsigned long paddr_t;

/// Internal definition of domain id.
typedef unsigned long domain_id_t;

/// Internal definition of index.
typedef unsigned long index_t;

/// Valid types for a capability.
typedef enum capability_type_t {
  Shared = 0,
  Confidential = 1,
  Other = 3,
} capability_type_t;

#endif
