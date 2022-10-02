#include <linux/types.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include "enclave.h"
#include "process.h"

#define PAGE 0x1000
// ——————————————————————— Globals to track enclaves ———————————————————————— //
static dll_list(struct enclave_t, enclaves);
static dll_list(struct pa_region_t, pages);

static int compare_encl(tyche_encl_handle_t first, tyche_encl_handle_t second)
{
  return first == second;
}

static int overlap(uint64_t s1, uint64_t e1, uint64_t s2, uint64_t e2)
{
  if ((s1 <= s2) && (s2 <= e1)) {
    return 1;
  }

  if ((s2 <= s1) && (s1 <= e2)) {
    return 1;
  }
  return 0;
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
  dll_init_elem(encl, list);
  // Add to the new enclave to the list.
  dll_add((&enclaves), encl, list);
  return 0;
}

/// Add a region to an existing enclave.
int add_region(struct tyche_encl_add_region_t* region)
{
  struct enclave_t* encl = NULL; 
  struct region_t* prev = NULL;
  struct region_t* e_reg = NULL;
  struct region_t* reg_iter = NULL;
  struct pa_region_t* page_iter = NULL;
  dll_foreach((&enclaves), encl, list) {
    if (encl->handle == region->handle)
      break;
  }
  // We could not find the enclave.
  if (!encl) {
    return -1;
  }
  // Check that the task calling is the one that created the enclave.
  if (encl->pid != current->pid) {
    pr_err("[TE]: Attempt to add a page to an enclave from a different task!\n");
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

  // Check that the full region is mapped and collect the relevant pages. 
  if (inspect_region(e_reg) != 0) {
    pr_err("[TE]: Invalid memory region.\n");
    goto failure;
  }

  // Check there is no overlap with other regions.
  dll_foreach((&encl->regions), reg_iter, list) {
    if (overlap(reg_iter->start, reg_iter->end, e_reg->start, e_reg->end)) {
      goto failure;
    } 
    if (reg_iter->end <= e_reg->start) {
      prev = reg_iter; 
    }
    if (reg_iter->start >= e_reg->end) {
      break;
    }
  }
  //TODO get the physical mappings.

  //Check physical pages are not already mapped in the enclave.
  dll_foreach((&e_reg->pas), page_iter, list) {
    struct pa_region_t* page_iter2 = NULL;
    struct pa_region_t* prev = NULL;
    pr_info("[TE]: Begin foreach\n");
    dll_foreach((&pages), page_iter2, globals) {
      if (overlap(page_iter->start, page_iter->end, page_iter2->start, page_iter2->end)) {
        pr_err("[TE]: the physical range is already used.\n");
        goto failure_remove;
      } 
      if (page_iter2->end <= page_iter->start) {
        prev = page_iter2;
      }
      if (page_iter2->start >= page_iter->end) {
        break;
      }
    }
    // Add the physical pages.
    if (prev) {
      dll_add_after((&pages), page_iter, globals, page_iter2);   
    } else {
      dll_add_first((&pages), page_iter, globals);
    } 
  } 
  //TODO generate the cr3
  
  // Add the region to the enclave
  if (prev) {
    dll_add_after((&encl->regions), e_reg, list, prev);
  } else {
    dll_add_first((&encl->regions), e_reg, list);
  }
  return 0;
failure_remove:
  if (page_iter == NULL) {
    pr_err("[TE]: page_iter should not be null here!\n");
    goto failure;
  }
  // Remove whatever we had added to the global list.
  for (page_iter = page_iter->globals.prev; page_iter != NULL;) {
    struct pa_region_t* page_save = page_iter->globals.prev; 
    dll_remove((&pages), page_iter, globals);
    page_iter = page_save;
  }

failure:
  kfree(e_reg);
  return -1;
}
