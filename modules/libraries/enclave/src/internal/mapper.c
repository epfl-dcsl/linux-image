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
failure_unmap:
  vfree(allocation);
  return NULL;
}

/// Create the tree for the enclave.
int build_enclave_cr3(struct enclave_t* encl) {
  pt_profile_t profile = x86_64_profile;
  map_info_t info = {&profile, encl};
  profile.allocate = allocate;
  profile.pa_to_va = pa_to_va;
  profile.va_to_pa = va_to_pa;
  profile.extras = (void*) &info;

  return 0;
}
