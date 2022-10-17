#include "tyche_capabilities.h"

// ———————————————————————————————— Globals ————————————————————————————————— //
static domain_t local_domain;

/// Copied from the tyche source code.
#define TYCHE_DOMAIN_GET_OWN_ID 0x100
#define TYCHE_DOMAIN_CREATE 0x101
#define TYCHE_DOMAIN_SEAL 0x102
#define TYCHE_DOMAIN_NB_REGION 0x103
#define TYCHE_DOMAIN_GET_REGION 0x104
#define TYCHE_REGION_SPLIT 0x200

// —————————————————————————————— Local types ——————————————————————————————— //

/// A type to pass arguments and receive when calling tyche.
typedef struct vmcall_frame_t {
  // Vmcall id.
  unsigned long id;

  // Arguments.
  unsigned long value_1;
  unsigned long value_2;
  unsigned long value_3;

  // Results.
  unsigned long ret_1;
  unsigned long ret_2;
  unsigned long ret_3;
  unsigned long ret_4;
} vmcall_frame_t;

/// Simple generic vmcall implementation.
static int tyche_call(vmcall_frame_t* frame)
{
  unsigned long result = 1;
  asm volatile(
    "movq %5, %%rax\n\t"
    "movq %6, %%rcx\n\t"
    "movq %7, %%rdx\n\n"
    "movq %8, %%rsi\n\t"
    "vmcall\n\t"
    "movq %%rax, %0\n\t"
    "movq %%rcx, %1\n\t"
    "movq %%rdx, %2\n\t"
    "movq %%rsi, %3\n\t"
    "movq %%rdi, %4"
    : "=rm" (result), "=rm" (frame->ret_1), "=rm" (frame->ret_2), "=rm" (frame->ret_3), "=rm" (frame->ret_4)
    : "rm" (frame->id), "rm" (frame->value_1), "rm" (frame->value_2), "rm" (frame->value_3)
    : "rax", "rcx", "rdx", "rsi", "memory");

  return (int)result;
} 

int init_domain(capa_alloc_t allocator, capa_dealloc_t deallocator)
{
  vmcall_frame_t frame;
  unsigned long nb_regions = 0;
  unsigned long i = 0;
  capability_t* capa = 0;
  if (allocator == 0) {
    return -1;
  }
  local_domain.alloc = allocator;
  local_domain.dealloc = deallocator;

  // Acquire the current domain's id.
  frame.id = TYCHE_DOMAIN_GET_OWN_ID;
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  local_domain.id = (index_t) frame.ret_1;
  // clean
  frame.ret_1 = 0;

  // Enumerate the regions for this domain.
  frame.id = TYCHE_DOMAIN_NB_REGION; 
  if (tyche_call(&frame) != 0) {
    return -1;
  }
  nb_regions = frame.ret_1; 
  // clean
  frame.ret_1 = 0;
  
  // For all the regions, ask tyche to give us bounds and tpe. 
  frame.id = TYCHE_DOMAIN_GET_REGION;
  for (i = 0; i < nb_regions; i++) {
    frame.value_1 = i; 
    if (tyche_call(&frame) != 0) {
      return -1;
    } 
    
    capa = (capability_t*)local_domain.alloc(sizeof(capability_t));
    if (capa == 0) {
      goto failure;
    }
    capa->id = frame.ret_1;
    capa->start = frame.ret_2;
    capa->end = frame.ret_3;
    capa->tpe = (capability_type_t) frame.ret_4;
    dll_init_elem(capa, list);
    dll_add(&(local_domain.capabilities), capa, list);
  }
  // All done.
  return 0;
failure:
  capa = 0;
  while(!dll_is_empty(&local_domain.capabilities)) {
    capa = local_domain.capabilities.head;
    dll_remove(&(local_domain.capabilities), capa, list);
    local_domain.dealloc((void*)capa);
  }
  return -1;
}

int transfer_capa(domain_id_t dom, paddr_t start, paddr_t end, capability_type_t tpe)
{
  return 0;
}

int merge_capa(domain_id_t owner, paddr_t start, paddr_t end, capability_type_t tpe)
{
  return 0;
}
