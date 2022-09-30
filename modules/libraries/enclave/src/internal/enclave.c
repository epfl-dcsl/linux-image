#include <linux/slab.h>
#include "enclave.h"

// ——————————————————————— Globals to track enclaves ———————————————————————— //
static dll_list(struct enclave_t, enclaves);

static int compare_encl(tyche_encl_handle_t first, tyche_encl_handle_t second)
{
  return first == second;
}

void enclave_init(void)
{
  dll_init_list((&enclaves));
}


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
  encl->handle = handle;
  // Add to the new enclave to the list.
  dll_add((&enclaves), encl, list);
  return 0;
}
