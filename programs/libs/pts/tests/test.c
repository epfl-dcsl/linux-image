#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "pts_api.h"
#include "x86_64_pt.h"


// ——————————————————————————————— Debugging ———————————————————————————————— //

#define LOG(...)                                          \
  do {                                                    \
    printf("[%s:%d] %s: ", __FILE__, __LINE__, __func__); \
    printf(__VA_ARGS__);                                  \
    printf("\n");                                         \
  } while (0);

// ————————————————————————— Page Table Simulation —————————————————————————— //
#define PTE_NB_ENTRIES 512
typedef struct pte_t {
  entry_t entries[PTE_NB_ENTRIES];
} pte_t;

#define ALLOC_NB_ENTRIES 2000
#define ALLOC_RAW_SIZE  (ALLOC_NB_ENTRIES * PAGE_SIZE)

/// Allocator of ptes that uses the pool above.
typedef struct pte_allocator_t {
  char* start;
  char* next_free;
  char* end; 
} pte_allocator_t;

/// Let's keep things easy, use a global allocator.
pte_allocator_t allocator = {0};

/// We just simualte segmentation from the allocator region.
addr_t pa_to_va(addr_t addr) {
  return  ((addr_t)allocator.start) + addr;
}

/// Again, we have segmentation.
/// Remove the base from the virtual address.
addr_t va_to_pa(addr_t addr) {
  TEST(addr >= ((addr_t)allocator.start));
  return addr - ((addr_t)allocator.start);
}

/// Stupid bump allocator.
entry_t* alloc(void* ptr) {
  TEST(allocator.next_free < allocator.end);
  entry_t* allocation = (entry_t*) allocator.next_free;
  allocator.next_free += PAGE_SIZE; 
  return (entry_t*) va_to_pa((addr_t)allocation);
}

/// Easy way to specify a virtual address.
typedef struct virt_addr_t {
  size_t pml4_idx;
  size_t pgd_idx;
  size_t pmd_idx;
  size_t pte_idx;
} virt_addr_t;

typedef struct extras_t {
  entry_t flags;
  size_t invoc_count;
} extras_t;

addr_t create_virt_addr(virt_addr_t virt)
{
  return (_AT(addr_t, virt.pml4_idx) << PML4_SHIFT) |
         (_AT(addr_t, virt.pgd_idx) << PGD_SHIFT) |
         (_AT(addr_t, virt.pmd_idx) << PMD_SHIFT) |
         (_AT(addr_t, virt.pte_idx) << PTE_SHIFT);
}

static void init_allocator()
{
  allocator.start = (char*) mmap(NULL, ALLOC_RAW_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  TEST(allocator.start != MAP_FAILED);
  allocator.next_free = allocator.start;
  allocator.end = allocator.start + ALLOC_RAW_SIZE;
  LOG("Done init allocator");
}

callback_action_t pte_page_mapper(entry_t* curr, level_t level, pt_profile_t* profile)
{
  TEST(curr != NULL);
  TEST(profile != NULL);
  TEST((*curr & _PAGE_PRESENT) == 0);
  TEST(profile->extras !=NULL);
  extras_t* extra = (extras_t*)(profile->extras);
  entry_t* new_page = profile->allocate(NULL);
  TEST((((entry_t)new_page) % PAGE_SIZE) == 0); 
  *curr = (((entry_t)new_page) & PHYSICAL_PAGE_MASK) | extra->flags;
  if (level == PTE) {
    extras_t* extras = (extras_t*) profile->extras;
    extras->invoc_count++;
  }
  return DONE; 
}

void test_simple_map(pt_profile_t* profile)
{
  // Let's just map 3 ptes as a start.
  virt_addr_t start = {0, 0, 0, 0};
  virt_addr_t end = {0, 0, 0, 3};
  addr_t s = create_virt_addr(start);
  addr_t e = create_virt_addr(end);
  extras_t extra = {__PP | _USR | __RW | __NX, 0};
  profile->extras = (void*) &extra;
  entry_t* root = profile->allocate(NULL); 
  walk_page_range((entry_t) root, PML4, s, e, profile);
  // Check that we mapped 3 ptes
  TEST(extra.invoc_count == 3);
  LOG("TEST simple map DONE");
  
}

int main(void) {
  LOG("TESTING X86_64 PTs");
  init_allocator();
  pt_profile_t my_profile = x86_64_profile;
  my_profile.how = x86_64_how_map;
  my_profile.mappers[PTE] = pte_page_mapper;
  my_profile.mappers[PMD] = pte_page_mapper;
  my_profile.mappers[PGD] = pte_page_mapper;
  my_profile.mappers[PML4] = pte_page_mapper;
  my_profile.allocate = alloc; 
  my_profile.pa_to_va = pa_to_va;
  my_profile.va_to_pa = va_to_pa;
  test_simple_map(&my_profile);
  return 0;
}
