#include "tyche_capabilities_types.h"
#include "tyche_capabilities.h"
#include "tyche_api.h"

// ———————————————————————————————— Globals ————————————————————————————————— //
domain_t local_domain;

// ———————————————————————— Private helper functions ———————————————————————— //
void local_memcpy(void* dest, void *src, unsigned long n) {
  char *csrc = (char*) src;
  char *cdest = (char*) dest;
  for (unsigned long i = 0; i < n; i++) {
    cdest[i] = csrc[i];
  }
}
// —————————————————————————————— Public APIs ——————————————————————————————— //


int init(capa_alloc_t allocator, capa_dealloc_t deallocator, capa_dbg_print_t print)
{
  capa_index_t i = 0;
  if (allocator == 0 || deallocator == 0 || print == 0) {
    goto fail;
  }
  // Set the local domain's functions.
  local_domain.alloc = allocator;
  local_domain.dealloc = deallocator;
  local_domain.print = print;
  local_domain.self = NULL;
  dll_init_list(&(local_domain.capabilities));
  dll_init_list(&(local_domain.children));

  // Start enumerating the domain's capabilities.
  // TODO identify self domain.
  for (i = 0; i < CAPAS_PER_DOMAIN; i++) {
    capability_t tmp_capa;
    if (enumerate_capa(i, &tmp_capa) != 0) {
      // Unable to read, move on. 
      continue; 
    };
    capability_t* capa = (capability_t*) (local_domain.alloc(sizeof(capability_t)));
    if (capa == NULL) {
      local_domain.print("Unable to allocate a capability!\n");
      goto failure;
    }
    // Copy the capability into the dynamically allocated one.
    local_memcpy(capa, &tmp_capa, sizeof(capability_t));
    dll_init_elem(capa, list);

    // Look for the self capability.
    if (capa->capa_type == Resource &&
        capa->resource_type == Domain &&
        capa->access.domain.status == Sealed) {
      if (local_domain.self != NULL) {
        // We have two sealed capabilities this doesn't make sense.
        local_domain.print("Found two sealed capa for the current domain.\n");
        goto failure;
      }
      local_domain.self = capa;
      // Do not add the self capability to the list.
      continue;
    }

    // Add the capability to the list.
    dll_add(&(local_domain.capabilities), capa, list);
  }
  return 0;
failure:
  while(!dll_is_empty(&local_domain.capabilities))
  {
    capability_t* capa = local_domain.capabilities.head;
    dll_remove(&(local_domain.capabilities), capa, list);
    local_domain.dealloc((void*)capa);
  }
fail: 
  return -1;
}

int create_domain(unsigned long spawn, unsigned long comm)
{
  capa_index_t new_self = -1;
  capability_t* child_capa = NULL;
  capability_t* revocation_capa = NULL; 
  child_domain_t* child = NULL;

  // Initialization was not performed correctly.
  if (local_domain.self == NULL) {
    goto fail;
  }

  // Perform allocations.
  child = (child_domain_t*) local_domain.alloc(sizeof(child_domain_t));
  if (child == NULL) {
    goto fail;
  }
  revocation_capa = (capability_t*) local_domain.alloc(sizeof(capability_t));
  if (revocation_capa == NULL) {
    goto fail_child;
  }
  child_capa = (capability_t*) local_domain.alloc(sizeof(capability_t));
  if (child_capa == NULL) {
    goto fail_revocation; 
  }

  // Create the domain.
  if (tyche_create_domain(
        &new_self, 
        &(child_capa->local_id),
        &(revocation_capa->local_id),
        spawn, comm) != 0) {
    goto fail;
  }

  // Populate the capabilities.
  if (enumerate_capa(new_self, local_domain.self) != 0) {
    goto fail_child_capa;
  }
  if (enumerate_capa(child_capa->local_id, child_capa) != 0) {
    goto fail_child_capa;
  }
  if (enumerate_capa(revocation_capa->local_id, revocation_capa) != 0) {
    goto fail_child_capa;
  }

  // Initialize the child domain.
  child->id = local_domain.id_counter++;
  child->revoke = revocation_capa;
  child->manipulate = child_capa; 
  dll_init_list(&(child->capabilities));
  dll_init_elem(child, list);

  // Add the child to the local_domain.
  dll_add(&(local_domain.children), child, list);

  // Add the capabilities to the local domain.
  dll_init_elem(revocation_capa, list);
  dll_init_elem(child_capa, list);
  dll_add(&(local_domain.capabilities), revocation_capa, list);
  dll_add(&(local_domain.capabilities), child_capa, list);

  // All done!
  return 0;

  // Failure paths.
fail_child_capa:
  local_domain.dealloc(child_capa);
fail_revocation:
  local_domain.dealloc(revocation_capa);
fail_child:
  local_domain.dealloc(child);
fail:
  return -1;
}
