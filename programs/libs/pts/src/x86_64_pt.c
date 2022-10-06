#include "pts_api.h"
#include "x86_64_pt.h"

/// Default profile for x86_64.
/// @warn It is incomplete.
const pt_profile_t x86_64_profile = {
  .nb_levels = x86_64_LEVELS,
  .nb_entries = PTRS_PER_PTE,
  .masks = {PTE_PAGE_MASK, PMD_PAGE_MASK, PGD_PAGE_MASK, PML4_PAGE_MASK},
  .shifts = {PTE_SHIFT, PMD_SHIFT, PGD_SHIFT, PML4_SHIFT},
  .how = x86_64_how_visit_leaves,
  .next = x86_64_next,
};

/// Example how function that asks to visit present leaves.
callback_action_t x86_64_how_visit_leaves(entry_t* entry, level_t level, pt_profile_t* profile)
{
  if ((*entry & __PP) != __PP) {
    return SKIP; 
  }
  // We have a Giant or Huge mapping, visit the node.
  if (level == PTE || ((level == PGD || level == PMD) &&
      ((*entry & _PAGE_PAT_LARGE) == _PAGE_PAT_LARGE))) {
    return VISIT;
  }
  return WALK;
}

/// Example how function that asks to map missing entries.
callback_action_t x86_64_how_map(entry_t* entry, level_t level, pt_profile_t* profile)
{
  if ((*entry & __PP) != __PP) {
    return MAP; 
  }
  return WALK;
}

index_t x86_64_get_index(addr_t addr, level_t level, pt_profile_t* profile)
{
  index_t idx = 0;
  // Clear the address
  addr = addr & VIRTUAL_PAGE_MASK;
  TEST(level <= x86_64_LEVELS);
  return ((addr & profile->masks[level]) >> profile->shifts[level]);
}

entry_t x86_64_next(entry_t entry, level_t curr_level) 
{
  return (entry & PHYSICAL_MASK);
}
