#include "ecs.h"
#include "tyche_api.h"

int read_header(ecs_header_t* header)
{
  vmcall_frame_t frame;
  if (header == NULL) {
    return -1;
  }
  frame.id = TYCHE_ECS_READ; 
  frame.value_1 = TYCHE_ECS_HDR_OFF; 
  frame.value_2 = TYCHE_ECS_HDR_SZ; 
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  header->size = frame.ret_1;
  header->head = frame.ret_2;
  header->tail = frame.ret_3;
  return 0;
}


//TODO it would be better if we had a way to write everything at once.
int write_header(ecs_header_t* header)
{
  vmcall_frame_t frame;
  if (header == NULL) {
    return -1;
  }
  frame.id = TYCHE_ECS_WRITE;
  frame.value_1 = TYCHE_ECS_HDR_OFF;
  frame.value_2 = header->size;

  if(tyche_call(&frame) != 0) {
    return -1;
  }
  frame.value_1++;
  frame.value_2 = header->head;
  if(tyche_call(&frame) != 0) {
    return -1;
  }
  frame.value_1++;
  frame.value_2 = header->tail;
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  return 0;
}

int read_entry(index_t idx, ecs_entry_t* entry)
{
  vmcall_frame_t frame;
  if (entry == NULL || idx < TYCHE_ECS_ENT_SZ) {
    return -1;
  }
  frame.id = TYCHE_ECS_READ;
  frame.value_1 = idx;
  frame.value_2 = TYCHE_ECS_ENT_SZ; 
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  entry->handle = frame.ret_1;
  entry->prev = frame.ret_2;
  entry->next = frame.ret_3;
  return 0;
}

int write_entry(index_t idx, ecs_entry_t* entry)
{
  vmcall_frame_t frame;
  if (entry == NULL || idx < TYCHE_ECS_HDR_SZ) {
    return -1;
  }
  frame.id = TYCHE_ECS_WRITE;
  frame.value_1 = idx;
  frame.value_2 = entry->handle;

  if(tyche_call(&frame) != 0) {
    return -1;
  }
  frame.value_1++;
  frame.value_2 = entry->prev;
  if(tyche_call(&frame) != 0) {
    return -1;
  }
  frame.value_1++;
  frame.value_2 = entry->next;
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  return 0;
}

int add_entry(ecs_header_t* header, index_t idx, ecs_entry_t* entry)
{
  ecs_entry_t tail;
  if (header == NULL || entry == NULL) {
    goto failure;
  }
  entry->prev = TYCHE_CAPA_NULL;
  entry->next = TYCHE_CAPA_NULL;
  if (header->head == TYCHE_CAPA_NULL) {
    header->head = idx;
    header->tail = idx;
  } else {
    if (read_entry(header->tail, &tail) != 0) {
      goto failure;
    }
    tail.next = idx;
    entry->prev = header->tail;
    if (write_entry(header->tail, &tail) != 0) {
      goto failure;
    }
    if (write_entry(idx, entry) != 0) {
      goto failure;
    }
    header->tail = idx;
  }
  header->size++;
  if (write_header(header) != 0) {
    goto failure;
  }
  return 0;
failure:
  return -1;
}
