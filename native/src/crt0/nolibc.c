#include "nolibc/crt.h"
#include "nolibc/arch.h"
#include "nolibc/stdio.h"

// errno

static int g_errno = 0;

int *__errno(void) {
    return &g_errno;
}

long __set_errno_internal(int n) {
    g_errno = n;
    return -1;
}
