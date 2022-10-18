#include "tyche_capabilities.h"
#include "tyche_api.h"

// ———————————————————————————————— Globals ————————————————————————————————— //
static domain_t local_domain;



// —————————————————————————————— Local types ——————————————————————————————— //

int init_domain(capa_alloc_t allocator, capa_dealloc_t deallocator)
{
  vmcall_frame_t frame;
  unsigned long nb_regions = 0;
  unsigned long i = 0;
  unsigned long handle = 0;
  capability_t* capa = 0;
  if (allocator == 0) {
    return -1;
  }
  local_domain.alloc = allocator;
  local_domain.dealloc = deallocator;

  // Acquire the current domain's id.
  frame.id = TYCHE_DOMAIN_GET_OWN_ID;
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  local_domain.id = (index_t) frame.ret_1;
  // clean
  frame.ret_1 = 0;

  // Enumerate the regions for this domain.
  frame.id = TYCHE_ECS_READ; 
  frame.value_1 = 0;
  frame.value_2 = 1;
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  nb_regions = frame.ret_1; 
  // clean
  frame.ret_1 = 0;
  
  // For all the regions, ask tyche to give us bounds and tpe. 
  frame.id = TYCHE_ECS_READ;
  for (i = 0; i < nb_regions; i++) {
    frame.value_1 = i;
    frame.value_2 = 1; 
    if (tyche_call(&frame) != 0) {
      goto failure;
    }
    // We got the handle as a result.
    handle = frame.ret_1; 
    
    // Let's read the capa information.
    frame.id = TYCHE_READ_CAPA;
    frame.value_1 = handle;
    if (tyche_call(&frame) != 0) {
      goto failure; 
    }
   
    // We might need to maintain a linked list of capabilities in tyche.
    capa = (capability_t*)local_domain.alloc(sizeof(capability_t));
    if (capa == 0) {
      goto failure;
    }
    capa->id = i;
    capa->start = frame.ret_1;
    capa->end = frame.ret_2;
    capa->tpe = (capability_type_t) frame.ret_3;
    dll_init_elem(capa, list);
    dll_add(&(local_domain.capabilities), capa, list);
  }
  // All done.
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
