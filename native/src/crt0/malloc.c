// Configure dlmalloc

#define USE_LOCKS           0
#define INSECURE            1
#define HAVE_MORECORE       0
#define NO_MALLINFO         1
#define NO_MALLOC_STATS     1
#define LACKS_TIME_H        1
#define malloc_getpagesize  4096

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-but-set-variable"

#include "dlmalloc/malloc.c"

#pragma clang diagnostic pop
