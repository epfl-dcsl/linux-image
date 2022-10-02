#include <linux/types.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include "process.h"

void inspect_region(struct region_t* region) {
  struct mm_struct *mm = NULL;
  struct vm_area_struct *vma = NULL;
  uint64_t address = 0;
  // Find the current process.
  printk(KERN_ALERT "[TE]: Calling process `%s` (%d)\n", current->comm, current->pid);
  for (address = region->start; address < region->end; address += PAGE_SIZE) {
    vma = find_vma(current->mm, address);
    if (vma == NULL) {
      printk(KERN_ALERT "[TE]: Unable to find mapping for %llx!\n", address);
    } else {
      printk(KERN_NOTICE "Found a VMA: %lx\n", vma->vm_flags);
    }
  } 
}
