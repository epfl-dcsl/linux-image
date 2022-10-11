#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
#include "mapper.h"


static addr_t va_to_pa(addr_t addr) {
  return (addr_t) virt_to_phys((void*) addr);
}

static addr_t pa_to_va(addr_t addr) {
  return (addr_t) phys_to_virt((phys_addr_t) addr);
}

static entry_t* allocate(void* ptr)
{
  map_info_t* info = NULL;
  void* allocation = NULL;
  struct pa_region_t* region = NULL;
  entry_t* page = NULL;

  info = (map_info_t*) ptr;
  if (info == NULL || info->profile == NULL || info->enclave == NULL) {
    return NULL;
  }
  
  allocation = __vmalloc(PT_PAGE_SIZE, GFP_KERNEL);
  if (allocation == NULL) {
    return NULL;
  }
  // Get the physical address.
  page = (entry_t*) virt_to_phys(allocation); 

  // Create a pa entry in the enclave.
  region = kmalloc(sizeof(struct pa_region_t), GFP_KERNEL);
  if (region == NULL) {
    goto failure_unmap;
  }
  // Populate the region's attributes.
  region->start = (uint64_t)(page);
  region->end = region->start + PT_PAGE_SIZE;
  region->tpe = PtEntry;
  dll_init_elem(region, list);
  dll_init_elem(region, globals);
  
  // Add the region to the enclave's page table.
  dll_add(&(info->enclave->pts), region, list);
  // return the value
  return page;
failure_unmap:
  vfree(allocation);
  return NULL;
}

callback_action_t default_mapper(entry_t* entry, level_t lvl, struct pt_profile_t* profile)
{
  map_info_t* info = NULL;
  entry_t* new_page = NULL;
  int is_huge = 0;
  entry_t size = 0;
  // There is something going wrong.
  if (entry == NULL || profile == NULL || profile->extras == NULL) {
    pr_err("default mapper received a null value.");
    return ERROR;
  }
  info = (map_info_t*)(profile->extras);
  if (info->pa_region == NULL) {
    return ERROR;
  }
  size = info->pa_region->end - info->pa_region->start;
  //TODO implement.
  switch(lvl) {
    case PT_PGD:
      // We have a huge mapping.
      if (size == PT_PGD_PAGE_SIZE) {
        is_huge = 1;
        goto entry_page; 
      }
      // Normal mapping.
      goto normal_page;
      break;
    case PT_PMD:
      if (size == PT_PMD_PAGE_SIZE) {
        is_huge = 1; 
        goto entry_page;
      }
      goto normal_page;
      break;
    case PT_PML4:
      // Easy case, just map the entry.
      goto normal_page;
    case PT_PTE:
      if (size != PT_PAGE_SIZE) {
        // There is a mismatch.
        return ERROR;
      }
      is_huge = 0;
      goto entry_page;
      break;
  }

// Mapping a page.
entry_page:
  *entry = (info->pa_region->start & PT_PHYS_PAGE_MASK)
    | info->region->flags;
  if (is_huge) {
    *entry |= PT_PAGE_PAT_LARGE;
  }
  // Move to the next region.
  info->pa_region = info->pa_region->list.next;
  return WALK;
// Allocating a new entry
normal_page:
  new_page = profile->allocate(profile->extras);
  *entry = (((entry_t)new_page) &PT_PHYS_PAGE_MASK) | info->intermed_flags;
  return WALK;
}

static int map_region(struct region_t* region, pt_profile_t* profile) {
  map_info_t* info = NULL;
  if (region == NULL || profile == NULL || profile->extras == NULL) {
    return -1;
  }
  info = (map_info_t*)(profile->extras);
  info->region = region;
  // We walk the physical page exactly in the same order we collected them.
  info->pa_region = region->pas.head;
  // Default flags for intermediary level mappings.
  info->intermed_flags = PT_PP | PT_RW | PT_ACC | PT_NX;
  return pt_walk_page_range(info->enclave->cr3, PT_PML4, region->start, region->end, profile);
}

/// Create the tree for the enclave.
int build_enclave_cr3(struct enclave_t* encl) {
  struct region_t* reg = NULL;
  pt_profile_t profile = x86_64_profile;
  map_info_t info = {
    .intermed_flags = 0,
    .region = NULL,
    .pa_region = NULL,
    .profile = &profile, 
    .enclave = encl,
  };
  profile.allocate = allocate;
  profile.pa_to_va = pa_to_va;
  profile.va_to_pa = va_to_pa;
  profile.extras = (void*) &info;
  profile.how = x86_64_how_map; 

  // Allocate the root.
  encl->cr3 = (uint64_t) allocate(&info);

  // The mappers.
  profile.mappers[PT_PTE] = default_mapper;
  profile.mappers[PT_PMD] = default_mapper;
  profile.mappers[PT_PGD] = default_mapper;
  profile.mappers[PT_PML4] = default_mapper;

  // Go through each region.
  dll_foreach(&(encl->regions), reg, list) {
    if (map_region(reg, &profile) != 0) {
      // There was a failure.
      return -1;
    }
  }
  //TODO do we need checks? 
  return 0;
}
