#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "tyche_capabilities.h"
// —————————————————————————————— Module Info ——————————————————————————————— //

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tyche team");
MODULE_DESCRIPTION("Tyche Capability driver LKM");
MODULE_VERSION("0.01");

// ———————————————————————————————— Helpers ————————————————————————————————— //

static void* local_allocator(unsigned long size)
{
  return kmalloc(size, GFP_KERNEL);
}

static void local_free(void* ptr)
{
  kfree(ptr);
}

// —————————————————————— Loading/Unloading  functions —————————————————————— //
static int __init tyche_capabilities_init(void)
{
  printk(KERN_INFO "Loading Tyche Capability LKM driver.");
  return init_domain(local_allocator, local_free);
}

static void __exit tyche_capabilities_exit(void)
{
  printk(KERN_INFO "Removing Tyche Capability LKM driver.");
}

// ————————————————————————————— API forwarders ————————————————————————————— //
int tyche_transfer_capability(domain_id_t dom, paddr_t start, paddr_t end, capability_type_t tpe)
{
  return transfer_capa(dom, start, end, tpe);
}
// ————————————————————————— Module's Registration —————————————————————————— //

module_init(tyche_capabilities_init);
module_exit(tyche_capabilities_exit);
EXPORT_SYMBOL(tyche_transfer_capability);
