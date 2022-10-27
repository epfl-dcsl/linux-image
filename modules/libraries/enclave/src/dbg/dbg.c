#include <linux/types.h>
#include <linux/sched.h>
#include <linux/pagewalk.h>
#include <linux/mm.h>
#include <linux/kprobes.h>
#include <linux/slab.h>
#include <asm/pgtable_types.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "x86_64_pt.h"

unsigned long debugging_cr3(void)
{
  void* cr3_virt = NULL;
  void* dest = NULL;
  int i = 0;
  unsigned long cr3_phys = 0;
  entry_t* root = NULL;
  cr3_virt = current->mm->pgd;
  cr3_phys = virt_to_phys(cr3_virt);
  printk(KERN_NOTICE "The current root v: %llx | p: %lx\n", (uint64_t)cr3_virt, cr3_phys);
  root = (entry_t*) phys_to_virt(cr3_phys | 0x1000);
  for (i = 0; i < 512; i++) {
    if ((root[i] & 0x1) == 0) {
      continue;
    }
    printk(KERN_NOTICE "[DUMP] %d: %llx\n", i, root[i]);
  }
  dest = alloc_pages_exact(PT_PAGE_SIZE, GFP_KERNEL);
  memcpy((void*)dest,(void*) root, PT_PAGE_SIZE);
  printk(KERN_NOTICE "The new root %llx | %llx\n", virt_to_phys(dest), PT_PAGE_SIZE);
  return virt_to_phys(dest);
}
