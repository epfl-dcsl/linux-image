#include <linux/types.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include "process.h"

//TODO figure out how to directly include from the kernel.
#define _PAGE_BIT_PRESENT	0	/* is present */
#define _PAGE_BIT_RW		1	/* writeable */
#define _PAGE_BIT_USER		2	/* userspace addressable */
#define _PAGE_BIT_PWT		3	/* page write through */
#define _PAGE_BIT_PCD		4	/* page cache disabled */
#define _PAGE_BIT_ACCESSED	5	/* was accessed (raised by CPU) */

int inspect_region(struct region_t* region) {
  struct mm_struct *mm = NULL;
  struct vm_area_struct *vma = NULL;
  uint64_t address = 0;
  // Find the current process.
  printk(KERN_ALERT "[TE]: Calling process `%s` (%d)\n", current->comm, current->pid);
  for (address = region->start; address < region->end; address += PAGE_SIZE) {
    vma = find_vma(current->mm, address);
    if (vma == NULL) {
      printk(KERN_ALERT "[TE]: Unable to find mapping for %llx!\n", address);
      return -1; 
    } else if ((vma->vm_page_prot.pgprot & _PAGE_BIT_USER) == 0) {
      printk(KERN_NOTICE "[TE]: The vma is not in user space %llx | %lx.\n", address, vma->vm_page_prot.pgprot);
      return -1;
    } else {
      printk(KERN_NOTICE "[TE]: Everything is fine %llx | %lx.\n", address, vma->vm_page_prot.pgprot);
    }
  } 
  return 0;
}
