#include "tyche_capabilities.h"
#include "tyche_api.h"

// ———————————————————————————————— Globals ————————————————————————————————— //
static domain_t local_domain;

// —————————————————————————————— Local types ——————————————————————————————— //

int init_domain(capa_alloc_t allocator, capa_dealloc_t deallocator)
{
  vmcall_frame_t frame;
  index_t i = 0;
  index_t capa_idx = 0;
  capability_t* capa = 0;
  if (allocator == 0) {
    return -1;
  }
  local_domain.alloc = allocator;
  local_domain.dealloc = deallocator;
  dll_init_list(&(local_domain.capabilities));
  dll_init_list(&(local_domain.frees));

  // Acquire the current domain's id.
  if (tyche_get_domain_id(&(local_domain.id)) != 0) {
    return -1;
  }
  
  // Read the header.
  if(read_header(&local_domain.hw) != 0) {
    return -1;
  }

  // Enumerate the regions for this domain.
  for (capa_idx = local_domain.hw.head; capa_idx != TYCHE_CAPA_NULL;) {
    // keep track of how many handles we visited.
    capa = (capability_t*)local_domain.alloc(sizeof(capability_t));
    if (capa == NULL) {
      goto failure;
    }
    if (read_entry(capa_idx, &(capa->hw)) != 0) {
      goto failure;
    }
    capa->index = capa_idx; 
    dll_init_elem(capa, list); 
    // Get the handle details.
    if (tyche_read_capa(capa->hw.handle, &(capa->start), &(capa->end), &(capa->tpe)) != 0) {
      goto failure;
    }
    
    // Add the capability to the list.
    dll_add(&(local_domain.capabilities), capa, list);
    capa_idx = capa->hw.next;
    i++;
  }
  if (i != local_domain.hw.size) {
    goto failure;
  }

  // Init the free lists now.
  for (i = TYCHE_ECS_FIRST_IDX; i < TYCHE_ECS_LAST_IDX; i+= TYCHE_ECS_STEP) {
    int contains = 0;
    capa = NULL;
    dll_foreach(&(local_domain.capabilities), capa, list) {
      if (capa->index == i) {
        contains = 1;
        break;
      }
    }
    if (contains != 0) {
      continue;
    }
    capa = NULL;
    capa = (capability_t*)local_domain.alloc(sizeof(capability_t));
    if (capa == NULL) {
      goto failure;
    }
    // The only valid element is i.
    capa->index = i;
    capa->start = 0;
    capa->end = 0;
    capa->tpe = Other;
    dll_add(&(local_domain.frees), capa, list);
  } 
  return 0;
failure:
  capa = 0;
  while(!dll_is_empty(&local_domain.capabilities)) {
    capa = local_domain.capabilities.head;
    dll_remove(&(local_domain.capabilities), capa, list);
    local_domain.dealloc((void*)capa);
  }
  while(!dll_is_empty(&local_domain.frees)) {
    capa = local_domain.frees.head;
    dll_remove(&(local_domain.frees), capa, list);
    local_domain.dealloc((void*)capa);
  }
  return -1;
}

capability_t* split_capa(capability_t* capa, paddr_t split_addr)
{
  capability_t *split = NULL;
  if (capa == NULL || split == NULL) {
    goto failure;
  }
  // Wrong capa.
  if (!dll_contains(capa->start, capa->end, split_addr)) {
    goto failure;
  }
  if (dll_is_empty(&(local_domain.frees)) != 0) {
    goto failure;
  }
  split = local_domain.frees.head;
  if (split == NULL) {
    goto failure;
  }
  dll_remove(&(local_domain.frees), local_domain.frees.head, list);
  split->start = split_addr;
  split->end = capa->end;
  split->tpe = capa->tpe;

  // Call tyche for the split.
  if (tyche_split_capa(capa->hw.handle, split_addr, &(split->hw.handle)) != 0) {
    goto failure;
  }
  
  // We managed to split!
  capa->end = split_addr;
  dll_add(&(local_domain.capabilities), split, list);

  // TODO Update the ecs.

  // TODO write the entry inside the ECS.
  return 0;
failure:
  return NULL;
}

int transfer_capa(domain_id_t dom, paddr_t start, paddr_t end, capability_type_t tpe)
{
  // TODO
  // 1. Find the capa. ok!
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
  return 0;
failure:
  return -1;
}

int merge_capa(domain_id_t owner, paddr_t start, paddr_t end, capability_type_t tpe)
{
  return 0;
}
