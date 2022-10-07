#include <linux/types.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include "enclave.h"
#include "mapper.h"
#include "process.h"

#define PAGE 0x1000
// ——————————————————————— Globals to track enclaves ———————————————————————— //
static dll_list(struct enclave_t, enclaves);

static int compare_encl(tyche_encl_handle_t first, tyche_encl_handle_t second)
{
  return first == second;
}

//If end == start, we do not consider it as an overlap.
static int overlap(uint64_t s1, uint64_t e1, uint64_t s2, uint64_t e2)
{
  if ((s1 <= s2) && (s2 < e1)) {
    goto fail;
  }

  if ((s2 <= s1) && (s1 < e2)) {
    goto fail;
  }
  return 0;
fail:
  printk(KERN_NOTICE "[TE]: 1: %llx - %llx ; 2: %llx - %llx\n", s1, e1, s2, e2);
  return 1;
}

static int region_check(struct tyche_encl_add_region_t* region)
{
  if (region == NULL) {
    goto failure;
  }
  // Check alignment.
  if (region->start % PAGE != 0 || region->end % PAGE != 0) {
    goto failure;
  }

  // Check ordering
  if (!(region->start < region->end)) {
    goto failure;
  }

  // Check access rights, we do not allow execute with write.
  if (region->flags == 0) {
    goto failure;
  }
  if ((region->flags & TE_EXEC) && (region->flags & TE_WRITE)) {
    goto failure;
  }

  if (region->tpe != Confidential && region->tpe != Shared) {
    goto failure;
  }

  return 1;

failure:
  return 0;
}

static struct enclave_t* find_enclave(tyche_encl_handle_t handle)
{
  struct enclave_t* encl = NULL; 
  dll_foreach((&enclaves), encl, list) {
    if (encl->handle == handle)
      break;
  }
  // We could not find the enclave.
  if (!encl) {
    pr_err("[TE]: Enclave not found in add_region.\n");
    return NULL;
  }
  // Check that the task calling is the one that created the enclave.
  if (encl->pid != current->pid) {
    pr_err("[TE]: Attempt to add a page to an enclave from a different task!\n");
    return NULL;
  }
  return encl;
}

void enclave_init(void)
{
  dll_init_list((&enclaves));
}

/// Add a new enclave.
/// The handle must be fresh.
int add_enclave(tyche_encl_handle_t handle)
{
  struct enclave_t* encl = NULL;
  // Check whether the enclave exists.
  dll_foreach((&enclaves), encl, list) {
    if (compare_encl(encl->handle, handle)) {
      // The enclave exists.    
      return -1;
    }
  }
  
  encl = kmalloc(sizeof(struct enclave_t), GFP_KERNEL);
  if (!encl) {
    // Failure to allocate.
    pr_err("[TE]: Failed to allocate new enclave!\n");
    return -1;
  }

  // Setup the handle.
  encl->pid = current->pid;
  encl->handle = handle;
  dll_init_list(&encl->regions);
  dll_init_list(&encl->pts);
  dll_init_list(&encl->all_pages);
  dll_init_elem(encl, list);
  // Add to the new enclave to the list.
  dll_add((&enclaves), encl, list);
  return 0;
}

/// Deletes all the physical regions in a region.
static void delete_region_pas(struct region_t* region)
{
  struct pa_region_t* curr = NULL;
  for (curr = region->pas.head; curr != NULL;) {
    struct pa_region_t* tmp = curr->list.next;
    kfree(curr);
    curr = tmp;
  }
}

/// Add a region to an existing enclave.
int add_region(struct tyche_encl_add_region_t* region)
{
  struct enclave_t* encl = NULL; 
  struct region_t* prev = NULL;
  struct region_t* e_reg = NULL;
  struct region_t* reg_iter = NULL;
  
  // Find the enclave.
  encl = find_enclave(region->handle);
  if (encl == NULL) {
    return -1;
  }
 
  // Lightweight checks.
  // Mappings to physical memory are checked later.
  if (!region_check(region))
  {
    pr_err("[TE]: Malformed region.\n");
    return -1;
  }

  // Allocate the region & set its attributes.
  e_reg = kmalloc(sizeof(struct region_t), GFP_KERNEL);
  if (!e_reg) {
    pr_err("[TE]: Failed to allocate a new region.\n");
    return -1;
  }
  e_reg->start = region->start;
  e_reg->end = region->end;
  e_reg->flags = region->flags;
  e_reg->tpe = region->tpe;
  dll_init_list(&e_reg->pas);
  dll_init_elem(e_reg, list); 

  // Check there is no overlap with other regions.
  // We keep the list sorted and attempt to merge regions whenever possible. 
  for (reg_iter = encl->regions.head; reg_iter != NULL;) {
    if (overlap(reg_iter->start, reg_iter->end, e_reg->start, e_reg->end)) {
      pr_err("[TE]: Virtual address overlap detected.\n");
      goto failure;
    } 

    // CASES WHERE: e_reg is on the left.

    // Too far in the list already and no merge.
    if (reg_iter->start > e_reg->end || (reg_iter->start == e_reg->end && reg_iter->tpe != e_reg->tpe)) {
      prev = reg_iter;
      break;
    }
    
    // Contiguous, we need to merge.
    // The second part of the && is redundant but makes code more readable.
    if( reg_iter->start == e_reg->end && reg_iter->tpe == e_reg->tpe) {
      reg_iter->start = e_reg->start;
      kfree(e_reg);
      e_reg = NULL;
      prev = NULL;
      break;
    }

    // CASES WHERE: e_reg is on the right.
   
    // Safely skip this entry.
    if (reg_iter->end < e_reg->start ||
      (reg_iter->end == e_reg->start && reg_iter->tpe != e_reg->tpe)) {
      goto next;
    }

    // We need to merge and have no guarantee that the next region does not
    // overlap.
    // Once again, the tpe check is redundant and is for readabilitty.
    if (reg_iter->end == e_reg->start && reg_iter->tpe == e_reg->tpe) {
      struct region_t* next = reg_iter->list.next;
      // There is an overlap with the next element.
      // We cannot add the region to the list.
      if (next != NULL && overlap(reg_iter->start, e_reg->end, next->start, next->end)){
        goto failure;
      }
      // Merge and remove.
      e_reg->start = reg_iter->start;
      dll_remove(&(encl->regions), reg_iter, list);
      kfree(reg_iter);
      break;
    }
    // We should never get here.
next:
    prev = reg_iter;
    reg_iter = reg_iter->list.next;
  }

  // The region has been merged.
  if (e_reg == NULL) {
    goto done;
  }

  if (prev != NULL) {
    dll_add_after(&encl->regions, e_reg, list, prev);
  } else {
    dll_add_first(&encl->regions, e_reg, list);
  }
done:
  return 0;
failure:
  kfree(e_reg);
  pr_err("[TE]: add_region failure.\n");
  return -1;
}

/// Done adding virtual regions to an enclave.
/// Commit the regions and find the corresponding physical pages.
/// @warn: Not the most efficient implementation, we go through the list
/// several times. We could merge all of this within the loop.
/// For now, keep it this way to ease debugging and readability.
int commit_enclave(tyche_encl_handle_t handle)
{
  struct enclave_t* encl = NULL; 
  struct region_t* region = NULL;
  encl = find_enclave(handle);
  if (encl == NULL) {
    return -1;
  }

  dll_foreach(&(encl->regions), region, list) {
    // Collect the physical pages.
    if (walk_and_collect_region(region) != 0) {
      goto failure; 
    }
  }
  //TODO create the cr3.
  if (build_enclave_cr3(encl)) {
    goto failure;
  }
  return 0;

  //TODO check for overlaps.
failure:
  // Delete all the pas.
  dll_foreach(&(encl->regions), region, list) {
    delete_region_pas(region);
  }
  return -1;
}

/// Adds a physical range to an enclave region.
/// This function keeps the list of region.pas sorted and attempts to merge
/// pas whenever possible. As a result, the pa_region might get freed and nullified. 
/// @COMMENT: The checks on the tpe are useless for now (as the tpe are always supposed to be the same).
/// I keep them just in case I manage to make this function reusable for other lists.
int add_pa_to_region(struct region_t* region, struct pa_region_t** pa_region) {
  struct pa_region_t* prev = NULL;
  struct pa_region_t* curr = NULL;
  
  // Easy case, the list is empty.
  if (dll_is_empty(&region->pas)) {
    dll_add(&region->pas, *pa_region, list); 
    return 0;
  }

  // Attempt to find a position in the list, handle merges
  for(curr = region->pas.head; curr != NULL;) {
    // Safety check first.
    if (overlap(curr->start, curr->end, (*pa_region)->start, (*pa_region)->end)) {
      return -1;
    }

    // CASES WHERE: pa_region is on the left.

    // Too far in the list already.
    if (curr->start > (*pa_region)->end ||
        (curr->start == (*pa_region)->end && curr->tpe != (*pa_region)->tpe)) {
      prev = curr;
      break;
    }
    
    // Contiguous, we are about to have some merging to do.
    if (curr->start == (*pa_region)->end && curr->tpe == (*pa_region)->tpe) {
      curr->start = (*pa_region)->start;
      kfree(*pa_region);
      *pa_region = NULL;
      prev = NULL;
      break;
    }

    // CASES WHERE: pa_region is on the right

    // Safely skip.
    if (curr->end < (*pa_region)->start ||
      (curr->end == (*pa_region)->start && curr->tpe != (*pa_region)->tpe)) {
      goto next;
    }

    // We need to merge, but have no guarantee that the next region does not
    // overlap.
    if (curr->end == (*pa_region)->start && curr->tpe == (*pa_region)->tpe) {
      struct pa_region_t* next = curr->list.next;
      if (next != NULL && overlap(curr->start, (*pa_region)->end, next->start, next->end)) {
        return -1;
      }
      (*pa_region)->start = curr->start;
      dll_remove(&region->pas, curr, list);
      kfree(curr);
      break;
    } 

next:
    prev = curr;
    curr = curr->list.next;
  }
  // The region has been merged.
  if (*pa_region == NULL) {
    goto done;
  }
  if (prev != NULL) {
    dll_add_after(&region->pas, *pa_region, list, prev); 
  }
  if (prev == NULL) {
    dll_add_first(&region->pas, *pa_region, list);
  }
done: 
  return 0;
}
