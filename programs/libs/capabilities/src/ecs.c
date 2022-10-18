#include "ecs.h"
#include "tyche_api.h"

int ecs_read_size(index_t* size)
{
  vmcall_frame_t frame;
  if (size == NULL) {
    goto fail;
  }
  frame.id = TYCHE_ECS_READ;
  frame.value_1 = TYCHE_ECS_SZ_IDX;
  frame.value_2 = 1;
  if (tyche_call(&frame) != 0) {
    goto fail;
  }
  *size = frame.ret_1;
  return 0;
fail:
  return -1;
}

int ecs_read_entry(index_t idx, paddr_t* entry)
{
  vmcall_frame_t frame;
  if (entry == NULL || idx == TYCHE_ECS_SZ_IDX) {
    goto fail;
  }
  frame.id = TYCHE_ECS_READ;
  frame.value_1 = idx;
  frame.value_2 = 1;
  if (tyche_call(&frame) != 0) {
    goto fail;
  }
  *entry = frame.ret_1;
  return 0;
fail:
  return -1;
}
