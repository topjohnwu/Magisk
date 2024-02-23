// Configure dlmalloc

#define USE_LOCKS           0
#define INSECURE            1
#define HAVE_MORECORE       0
#define NO_MALLINFO         1
#define NO_MALLOC_STATS     1

#include "dlmalloc/malloc.c"
