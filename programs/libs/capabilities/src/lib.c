#include "tyche_capabilities.h"
#include "tyche_api.h"

// ———————————————————————————————— Globals ————————————————————————————————— //
static domain_t local_domain;

// —————————————————————————————— Local types ——————————————————————————————— //

int init_domain(capa_alloc_t allocator, capa_dealloc_t deallocator)
{
  index_t i = 0;
  index_t nb_capa = 0;
  if (allocator == 0) {
    return -1;
  }
  local_domain.alloc = allocator;
  local_domain.dealloc = deallocator;
  dll_init_list(&(local_domain.capabilities));

  // Acquire the current domain's id.
  if (tyche_get_domain_id(&(local_domain.id)) != 0) {
    goto fail;
  }
  
  // Read the number of ECS entries.
  if (ecs_read_size(&nb_capa) != 0) {
    goto fail;
  }

  // Enumerate the regions for this domain.
  for (i = TYCHE_ECS_FIRST_ENTRY; i <= nb_capa; i++) {
    capability_t* capa = (capability_t*) local_domain.alloc(sizeof(capability_t));
    if (capa == NULL) {
      goto failure;
    }
    if (ecs_read_entry(i, &(capa->handle)) != 0) {
      goto failure;
    }
    capa->index = i; 
    dll_init_elem(capa, list); 
    // Get the handle details.
    if (tyche_read_capa(capa->handle, &(capa->start), &(capa->end), &(capa->tpe)) != 0) {
      goto failure;
    }
    
    // Add the capability to the list.
    dll_add(&(local_domain.capabilities), capa, list);
  }
  // We are all done
  return 0;
failure:
  while(!dll_is_empty(&local_domain.capabilities)) {
    capability_t* capa = local_domain.capabilities.head;
    dll_remove(&(local_domain.capabilities), capa, list);
    local_domain.dealloc((void*)capa);
  }
fail:
  return -1;
}

capability_t* split_capa(capability_t* capa, paddr_t split_addr)
{
  capability_t *split = NULL;
  if (capa == NULL) {
    goto failure;
  }
  // Wrong capability.
  if (!dll_contains(capa->start, capa->end, split_addr)) {
    goto failure;
  }

  // Allocate the new capability.
  split = (capability_t*) local_domain.alloc(sizeof(capability_t));
  if (split == NULL) {
    goto failure;
  }
  split->start = split_addr;
  split->end = capa->end;
  split->tpe = capa->tpe;
  dll_init_elem(split, list);

  // Call tyche for the split.
  if (tyche_split_capa(capa->handle, split_addr, &(split->handle)) != 0) {
    goto fail_dealloc;
  }
  
  // We managed to split!
  capa->end = split_addr;
  dll_add(&(local_domain.capabilities), split, list);
  return split;
fail_dealloc:
  local_domain.dealloc((void*)split);
failure:
  return NULL;
}

int transfer_capa(domain_id_t dom, paddr_t start, paddr_t end, capability_type_t tpe)
{
  // 1. Find the capa.
  // 2. Split.
  // 3. If tpe == Confidential -> remove
  // 4. call the transfer.
  capability_t* curr = NULL;
  capability_t* split = NULL;
  capability_t* rest = NULL;
  // Quick checks. TODO check alignment of start and end too.
  if (start >= end || tpe > Other
      || ((start % ALIGNMENT) != 0) || ((end % ALIGNMENT) != 0)) {
    goto failure;
  }
  dll_foreach(&(local_domain.capabilities), curr, list) {
    if (dll_contains(curr->start, curr->end, start)
        && dll_contains(curr->start, curr->end, end)) {
      // We found the region.
      break;
    }
  }

  // Unable to find the capa.
  if (curr == NULL) {
    goto failure;
  }
  
  // Split the capability.
  split = split_capa(curr, start);
  if (split == NULL) {
    goto failure;
  }

  // Do we need a three way split?
  if (split->end > end) {
    if (split_capa(split, end) == NULL) {
      goto failure;
    }
  } 

  // Now transfer with the right tpe.
  if (tpe <= Confidential) {
    tyche_grant_capa(dom, split->handle, tpe);
    dll_remove(&(local_domain.capabilities), split, list);
    local_domain.dealloc((void*)split);
  } else {
    tyche_share_capa(dom, split->handle, tpe);
  }
  return 0;
failure:
  return -1;
}

int merge_capa(domain_id_t owner, paddr_t start, paddr_t end, capability_type_t tpe)
{
  capability_t* curr = NULL;
  capability_t* merge = NULL;
  dll_foreach(&(local_domain.capabilities), curr, list) {
    if (dll_overlap(curr->start, curr->end, start, end) != 0) {
      goto failure;
    }
  }
  return 0;
failure:
  return -1;
}
