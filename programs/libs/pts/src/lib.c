#include "pts_api.h"

#ifdef DEBUG_USER_SPACE
#include <stdio.h>

#define TEST(cond)                                                   \
  do {                                                               \
    if (!(cond)) {                                                   \
      fprintf(stderr, "[%s:%d] %s\n", __FILE__, __LINE__, __func__); \
      abort();                                                       \
    }                                                                \
  } while (0);
#else
#define TEST(cond)  \
  do {              \
    if (!(cond)) {  \
      return -1;    \
    }               \
  } while(0);
#endif

int walk_page_range(entry_t root, level_t level, addr_t start, addr_t end, pt_profile_t* profile)
{
  TEST(start < end); 
  TEST(profile != 0);
  //TEST(root != 0);
  TEST(level < profile->nb_levels); 
  TEST(profile->how != 0);
  TEST(profile->get_index != 0);
  TEST(profile->pa_to_va != 0);
  TEST(profile->subrange != 0);
  entry_t next = 0;
  addr_t sub_start = 0, sub_end = 0;
  index_t s = profile->get_index(start, level);
  index_t e = profile->get_index(end, level);
  entry_t* va_root = (entry_t*) profile->pa_to_va(root);
  for (index_t i = s; i < e; i++) {
    // Decide what to do first.
    // We can either MAP -> WALK -> VISIT
    switch(profile->how(va_root[i], level, profile)) {
      // Skip this entry.
      case SKIP:
        continue;
        break;
      // VISIT this entry.
      case VISIT:
        goto visit;
        break;
      // Map this entry.
      case MAP:
        goto map;
        break;
      default:
        TEST(0);
        break;
    } 
map:
    // Add a mapping.
    if (profile->mappers[level] != 0) {
      switch(profile->mappers[level](va_root[i], level, profile)) {
        // That means we should not walk.
        case DONE:
          continue;
          break;
        case VISIT:
          goto visit;
          break;
        // Should never happen.
        case MAP:
        default:
          TEST(0);
          break;
      }
    }
visit:
    // Walk the mapping.
    if (profile->visitors[level] != 0) {
      switch(profile->visitors[level](va_root[i], level, profile)) {
        // The only acceptable return values.
        case DONE:
          // Recursive walk to the next level.
          goto walk;
          break;
        // Skip that entry.
        case SKIP:
          continue;
          break;
        default:
          TEST(0);
          break;
      }
    }
walk:
    // No more to do.
    if (level == 0) {
      continue;
    } 
    next = profile->next(va_root[i], level);
    profile->subrange(start, end, level, i, &sub_start, &sub_end); 
    if (walk_page_range(next, level-1, sub_start, sub_end, profile) == -1) {
      TEST(0);
      return -1; 
    }
  }
  return 0;
}
