#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdio.h>
#include <stdlib.h>

#define TEST(cond)                                                   \
  do {                                                               \
    if (!(cond)) {                                                   \
      fprintf(stderr, "[%s:%d] %s\n", __FILE__, __LINE__, __func__); \
      abort();                                                       \
    }                                                                \
  } while (0);

#endif
