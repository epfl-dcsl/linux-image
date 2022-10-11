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
  // There is something going wrong.
  if (entry == NULL || profile == NULL || profile->extras == NULL) {
    pr_err("default mapper received a null value.");
    return SKIP;
  }
  info = (map_info_t*)(profile->extras);
  //TODO implement.
  switch(lvl) {
    // These are the complicated cases.
    // We want to optimize for Giant and Huge regions, but this is doable
    // only if we have a contiguous physical region for it.
    case PT_PGD:
    case PT_PMD:
      // Do huge mapping only if possible. 
    case PT_PML4:
      // Easy case, just map the entry.
       new_page = profile->allocate(NULL);
      *entry = (((entry_t)new_page) &PT_PHYS_PAGE_MASK) | info->intermed_flags;
      return WALK;
    case PT_PTE:
      //TODO get the page mapping from the mapping.
      break;
  }
  return WALK;
}

static int map_region(struct region_t* region, pt_profile_t* profile) {
  map_info_t* info = NULL;
  if (region == NULL || profile == NULL || profile->extras == NULL) {
    return -1;
  }
  info = (map_info_t*)(profile->extras);
  //TODO set the flags now!
  //TODO set extra info in the profile.
  return pt_walk_page_range(info->cr3, PT_PML4, region->start, region->end, profile);
}

/// Create the tree for the enclave.
int build_enclave_cr3(struct enclave_t* encl) {
  struct region_t* reg = NULL;
  pt_profile_t profile = x86_64_profile;
  map_info_t info = {
    .cr3 = 0,
    .intermed_flags = 0,
    .entry_flags = 0,
    .profile = &profile, 
    .enclave = encl,
  };
  profile.allocate = allocate;
  profile.pa_to_va = pa_to_va;
  profile.va_to_pa = va_to_pa;
  profile.extras = (void*) &info;
  profile.how = x86_64_how_map; 

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

  //TODO do we need checks 
  return 0;
}
