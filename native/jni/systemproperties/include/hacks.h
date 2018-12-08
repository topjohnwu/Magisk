#pragma once

#include <stdio.h>

#ifndef PR_SET_VMA
#define PR_SET_VMA 0x53564d41
#endif
#ifndef PR_SET_VMA_ANON_NAME
#define PR_SET_VMA_ANON_NAME 0
#endif
#define getline __getline
#define fsetxattr __fsetxattr
extern "C" ssize_t __getline(char **, size_t *, FILE *);
extern "C" int __fsetxattr(int, const char *, const void *, size_t, int);
