#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/types.h>
#include <stdio.h>

#include "magisk.h"

// xwrap.c

FILE *xfopen(const char *pathname, const char *mode);

// vector.c

#include "vector.h"

// log_monitor.c

void monitor_logs();

#endif