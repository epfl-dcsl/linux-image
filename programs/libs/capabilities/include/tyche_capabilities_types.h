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
  ConfidentialRO = 1,
  ConfidentialRW = 2,
  ConfidentialRX = 3,
  Confidential = 4,
  SharedRO = 5,
  SharedRW = 6,
  SharedRX = 7,
  Shared = 8,
  Max = 8,
} capability_type_t;

/// Predefined values for capabilities
#define CAPA_NONE ((unsigned long)0)
#define CAPA_READ ((unsigned long)1 << 0)
#define CAPA_WRITE ((unsigned long)1 << 1)
#define CAPA_EXEC ((unsigned long)1 << 2)
#define CAPA_REVOK ((unsigned long)1 << 3)
#define CAPA_MAX CAPA_REVOK

#endif
