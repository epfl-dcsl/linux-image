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

void local_memset(void* dest, unsigned long n) {
  char *cdest = (char*) dest;
  for (unsigned long i = 0; i < n; i++) {
    cdest[i] = 0;
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
  return SUCCESS;
failure:
  while(!dll_is_empty(&local_domain.capabilities))
  {
    capability_t* capa = local_domain.capabilities.head;
    dll_remove(&(local_domain.capabilities), capa, list);
    local_domain.dealloc((void*)capa);
  }
fail: 
  return FAILURE;
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
  revocation_capa->left = NULL;
  revocation_capa->right = child_capa;
  child_capa->parent = revocation_capa;

  // Add the child to the local_domain.
  dll_add(&(local_domain.children), child, list);

  // Add the capabilities to the local domain.
  dll_init_elem(revocation_capa, list);
  dll_init_elem(child_capa, list);
  dll_add(&(local_domain.capabilities), revocation_capa, list);
  dll_add(&(local_domain.capabilities), child_capa, list);

  // All done!
  return SUCCESS;

  // Failure paths.
fail_child_capa:
  local_domain.dealloc(child_capa);
fail_revocation:
  local_domain.dealloc(revocation_capa);
fail_child:
  local_domain.dealloc(child);
fail:
  return FAILURE;
}

int duplicate_capa(
    capability_t** left,
    capability_t** right,
    capability_t* capa,
    unsigned long a1_1,
    unsigned long a1_2,
    unsigned long a1_3,
    unsigned long a2_1,
    unsigned long a2_2,
    unsigned long a2_3) {
  if (left == NULL || right == NULL || capa == NULL) {
    goto failure;
  }

  // Attempt to allocate left and right.
  *left = (capability_t*) local_domain.alloc(sizeof(capability_t));
  if (*left == NULL) {
    goto failure;
  }
  *right = (capability_t*) local_domain.alloc(sizeof(capability_t));
  if (*right == NULL) {
    goto fail_left;
  }

  // Call duplicate.
  if (tyche_duplicate(
        &((*left)->local_id), &((*right)->local_id), capa->local_id,
        a1_1, a1_2, a1_3, a2_1, a2_2, a2_3 ) != SUCCESS) {
    goto fail_right; 
  }

  // Update the capability.
  if (enumerate_capa(capa->local_id, capa) != SUCCESS) {
    local_domain.print("We failed to enumerate the root of a duplicate!");
    goto fail_right;
  }
  capa->left = *left;
  capa->right = *right;
  
  // Initialize the left.
  if(enumerate_capa((*left)->local_id, *left) != SUCCESS) {
    local_domain.print("We failed to enumerate the left of duplicate!");
    goto fail_right;
  }
  dll_init_elem((*left), list);
  dll_add(&(local_domain.capabilities), (*left), list); 
  (*left)->parent = capa;
  (*left)->left = NULL;
  (*left)->right = NULL;

  // Initialize the right.
  if (enumerate_capa((*right)->local_id, (*right)) != SUCCESS) {
    local_domain.print("We failed to enumerate the right of duplicate!");
    goto fail_right;
  }
  dll_init_elem((*right), list);
  dll_add(&(local_domain.capabilities), (*right), list);
  (*right)->parent = capa;
  (*right)->left = NULL;
  (*right)->right = NULL;

  // All done!
  return SUCCESS;

  // Failure paths.
fail_right:
  local_domain.dealloc(*right);
fail_left:
  local_domain.dealloc(*left);
failure:
  return FAILURE;
}

//TODO: for the moment only handle the case where the region is fully contained
//within one capability.
int grant_region(domain_id_t id, paddr_t start, paddr_t end, memory_access_right_t access) {
  child_domain_t* child = NULL;
  capability_t* capa = NULL;

  // Quick checks.
  if (start >= end) {
    local_domain.print("Error[grant_region]: start is greater or equal to end.\n");
    goto failure;
  }

  // Find the target domain.
  dll_foreach(&(local_domain.children), child, list) {
    if (child->id == id) {
      // Found the right one.
      break;
    }
  }

  // We were not able to find the child.
  if (child == NULL) {
    local_domain.print("Error[grant_region]: child not found."); 
    goto failure;
  }

  // Now attempt to find the capability.
  dll_foreach(&(local_domain.capabilities), capa, list) {
    if (capa->capa_type != Resource || capa->resource_type != Region) {
      continue;
    }
    if ((dll_contains(capa->access.region.start, capa->access.region.end, start)) && 
        capa->access.region.start <= end && capa->access.region.end >= end) {
      // Found the capability.
      break;
    }
  }

  // We were not able to find the capability.
  if (capa == NULL) {
    goto failure;
  }

  // The region is in the middle, requires two splits.
  if (capa->access.region.start < start && capa->access.region.end > end) {
    // Middle case.
    // capa: [s.......................e]
    // grant:     [m.....me]
    // Duplicate such that we have [s..m] [m..me..e]
    // And then update capa to be equal to the right
    capability_t* left = NULL, *right = NULL;
    if (duplicate_capa(&left, &right, capa,
          capa->access.region.start, start, capa->access.region.flags,
          start, capa->access.region.end, capa->access.region.flags) != SUCCESS) {
      goto failure;
    }
    // Update the capa to point to the right.
    capa = right;
  }

  // Requires a duplicate.
  if ((capa->access.region.start == start && capa->access.region.end > end) ||
      (capa->access.region.start < start && capa->access.region.end == end)) {
    paddr_t s = 0, m = 0, e = 0;
    capability_t* left = NULL, *right = NULL;
    capability_t** to_grant = NULL; 

    if (capa->access.region.start == start && capa->access.region.end > end) {
      // Left case.
      // capa: [s ............e].
      // grant:[s.......m].
      // duplicate: [s..m] - [m..e]
      // Grant the first portion.
      s = capa->access.region.start;
      m = end;
      e = capa->access.region.end;
      to_grant = &left;
    } else {
      // Right case.
      // capa: [s ............e].
      // grant:      [m.......e].
      // duplicate: [s..m] - [m..e]
      // Grant the second portion.
      s = capa->access.region.start;
      m = start;
      e = capa->access.region.end;
      to_grant = &right;
    }

    // Do the duplicate.
    if (duplicate_capa(&left, &right, capa, s, m, capa->access.region.flags,
          m, e, capa->access.region.flags) != SUCCESS) {
      goto failure;
    }
   
    // Now just update the capa to grant.
    capa = *to_grant;
  } 

  // At this point, capa should be a perfect overlap.
  if (capa == NULL || 
      !(capa->access.region.start == start && capa->access.region.end == end)) {
    goto failure;
  }

grant:
  if (tyche_grant(child->manipulate->local_id, capa->local_id,
        start, end, (unsigned long) access) != SUCCESS) {
    goto failure;
  }

  // Now we update the capabilities.
  dll_remove(&(local_domain.capabilities), capa, list);
  dll_add(&(child->capabilities), capa, list);

  // We are done!
  return SUCCESS;
failure:
  return FAILURE;
}


int share_region(
    domain_id_t id, paddr_t start, paddr_t end, memory_access_right_t access) {
  child_domain_t* child = NULL;
  capability_t* capa = NULL, *left = NULL;

  // Quick checks.
  if (start >= end) {
    local_domain.print("Error[grant_region]: start is greater or equal to end.\n");
    goto failure;
  }

  // Find the target domain.
  dll_foreach(&(local_domain.children), child, list) {
    if (child->id == id) {
      // Found the right one.
      break;
    }
  }

  // We were not able to find the child.
  if (child == NULL) {
    local_domain.print("Error[grant_region]: child not found."); 
    goto failure;
  }

  // Now attempt to find the capability.
  dll_foreach(&(local_domain.capabilities), capa, list) {
    if (capa->capa_type != Resource || capa->resource_type != Region) {
      continue;
    }
    // TODO check dll_contains, might fail on end.
    if (dll_contains(capa->access.region.start, capa->access.region.end, start) && 
        capa->access.region.start <= end &&  capa->access.region.end >= end) {
      // Found the capability.
      break;
    }
  }

  // We were not able to find the capability.
  if (capa == NULL) {
    goto failure;
  }

  // A share is less complex than a grant because we do not need to carve out
  // the address space, but we need to keep track of how to merge things back.
  left = (capability_t*) local_domain.alloc(sizeof(capability_t)); 
  if (left == NULL) {
    goto failure;
  }
  if (tyche_share(&(left->local_id), child->manipulate->local_id,
        capa->local_id, start, end, (unsigned long) access) != SUCCESS) {
    goto fail_left;
  }
  
  // Now we update the capabilities.
  if (enumerate_capa(capa->local_id, capa) != SUCCESS) {
    local_domain.print("We failed enumerating the revocation after share.");
    goto fail_left;
  }
  dll_remove(&(local_domain.capabilities), capa, list);
  dll_add(&(child->capabilities), capa, list);
  capa->left = left;
  capa->right = NULL;

  if (enumerate_capa(left->local_id, left)) {
    local_domain.print("We failed enumerating the remainder of the share.");
    goto fail_left;
  }
  left->left = NULL;
  left->right = NULL;
  left->parent = capa;
  dll_add(&(local_domain.capabilities), left, list);

fail_left:
  local_domain.dealloc(left);
failure:
  return FAILURE;
}
