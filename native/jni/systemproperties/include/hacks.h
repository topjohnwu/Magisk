#pragma once

#include <stdio.h>
#include <syscall.h>

#ifndef PR_SET_VMA
#define PR_SET_VMA 0x53564d41
#endif
#ifndef PR_SET_VMA_ANON_NAME
#define PR_SET_VMA_ANON_NAME 0
#endif
#define getline __getline
#define fsetxattr(...) syscall(__NR_fsetxattr, __VA_ARGS__)
extern "C" ssize_t __getline(char **, size_t *, FILE *);
