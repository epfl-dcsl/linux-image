#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/smp.h>
// —————————————————————————————— Module Info ——————————————————————————————— //

MODULE_AUTHOR("Tyche team");
MODULE_DESCRIPTION("Tyche IPI Test");
MODULE_VERSION("0.01");

// ———————————————————————————————— Helpers ————————————————————————————————— //

void ipi_test(void *data)
{
    pr_info("CPU %d: Hello world from ipi test\n", smp_processor_id());
}

// —————————————————————— Loading/Unloading  functions —————————————————————— //
static int __init init_ipi_test(void)
{
    preempt_disable();
    pr_info("CPU %d init_ipi_test\n", smp_processor_id());
    preempt_enable();

    smp_call_function(ipi_test, NULL, 1 /* Wait for completion */);

    return 0;
}

static void __exit cleanup_ipi_test(void)
{
    pr_info("ipi test done\n");
}

// ————————————————————————— Module's Registration —————————————————————————— //

module_init(init_ipi_test);
module_exit(cleanup_ipi_test);
MODULE_LICENSE("GPL");
