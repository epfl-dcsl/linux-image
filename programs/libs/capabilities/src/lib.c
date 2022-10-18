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
  return 0;
failure:
  capa = 0;
  while(!dll_is_empty(&local_domain.capabilities)) {
    capa = local_domain.capabilities.head;
    dll_remove(&(local_domain.capabilities), capa, list);
    local_domain.dealloc((void*)capa);
  }
  return -1;
}

int transfer_capa(domain_id_t dom, paddr_t start, paddr_t end, capability_type_t tpe)
{
  return 0;
}

int merge_capa(domain_id_t owner, paddr_t start, paddr_t end, capability_type_t tpe)
{
  return 0;
}
