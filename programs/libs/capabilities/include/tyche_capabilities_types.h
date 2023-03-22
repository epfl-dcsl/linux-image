#ifndef __INCLUDE_TYCHE_CAPABILITIES_TYPES_H__
#define __INCLUDE_TYCHE_CAPABILITIES_TYPES_H__

#ifndef NULL
#define NULL ((void*)0)
#endif

#include "dll.h"

#define SUCCESS (0)
#define FAILURE (-1)

/// Internal definition of our types so we can move to 32 bits.
typedef long long unsigned int paddr_t;

/// Internal definition of domain id.
typedef unsigned long domain_id_t;

/// Internal definition of index.
typedef unsigned long capa_index_t;

/// Mirrors the types defined in crates/capabilities/src/lib.rs.
typedef enum capa_type_t {
  Resource = 0,
  Revocation = 1 << 3,
} capa_type_t;

/// Mirrors the various types of resources handled by the capabilities.
typedef enum capa_rtype_t {
  Domain = 1 << 0,
  Region = 1 << 1,
  CPU = 1 << 2,
} capa_rtype_t;

/// Status of a domain capability.
typedef enum domain_status_t {
  None = 0,
  Unsealed = 1 << 0,
  Sealed = 1 << 1,
  Channel = 1 << 2,
  Transition = 1 << 3,
  // These are used for the ABI calls
  Spawn = 1 << 4,
  Comm = 1 << 5,
} domain_status_t;

/// Region Access Rights
typedef enum memory_access_right_t {
  MEM_READ = 1 << 0,
  MEM_WRITE = 1 << 1,
  MEM_EXEC = 1 << 2,
  MEM_SUPER = 1 << 3,
  MEM_SHARE = 1 << 4,
} memory_access_right_t;

/// Access right information for a region capability.
typedef struct capa_region_t {
  paddr_t start;
  paddr_t end;
  paddr_t flags;
} capa_region_t;

/// Access right information for a cpu capability.
typedef struct capa_cpu_t {
  paddr_t flags;
} capa_cpu_t;

/// Access right information for a domain capability.
typedef struct capa_domain_t {
  domain_status_t status;
  union {
    // If the status is Sealed or Unsealed.
    struct {
      char spawn;
      char comm;
    } capas;
    // If the status is Transition.
    int transition;
  } info;
} capa_domain_t;

/// A capability can be any of these three types.
typedef union capa_descriptor_t {
  capa_domain_t domain;
  capa_region_t region;
  capa_cpu_t cpu;
} capa_descriptor_t;

/// Capability that confers access to a memory region.
typedef struct capability_t {
  // General capability information.
  capa_index_t local_id;
  capa_type_t capa_type;
  capa_rtype_t resource_type;
  capa_descriptor_t access;

  // This is stored for convenience but might not be up-to-date.
  unsigned long last_read_ref_count;

  // Tree structure.
  struct capability_t* parent;
  struct capability_t* left;
  struct capability_t* right;

  // This structure can be put in a double-linked list
  dll_elem(struct capability_t, list);
} capability_t;

typedef void* (*capa_alloc_t)(unsigned long size);
typedef void (*capa_dealloc_t)(void* ptr);
typedef void (*capa_dbg_print_t)(const char* msg);

/// Represents the current domain's metadata.
typedef struct domain_t {
  // Allocate ids from this counter for children domains.
  domain_id_t id_counter;

  // reference to ourselves.
  capability_t* self;

  // The allocator to use whenever we need a new structure.
  capa_alloc_t alloc;
  capa_dealloc_t dealloc;
  capa_dbg_print_t print;

  // All the children for this domain.
  dll_list(struct child_domain_t, children);

  // The list of used capabilities for this domain.
  dll_list(struct capability_t, capabilities);
} domain_t;

/// Represents a child domain.
/// We keep track of:
/// 1) The main communication channel.
/// 2) The revocation handle to kill the domain.
/// 3) All the resources we passed to the domain.
typedef struct child_domain_t {
  // The domain's local id.
  domain_id_t id;

  // Handle to the domain, this would be first an unsealed than a channel.
  capability_t* manipulate;

  // Handle to revoke the domain after a seal.
  capability_t* revoke;

  // All the revocations for resources passed to the domain.
  dll_list(struct capability_t, capabilities);

  // All the transition handles to this domain.
  dll_list(struct capability_t, transitions);

  // This structure can be put in a double-linked list.
  dll_elem(struct child_domain_t, list);
} child_domain_t;

#endif
