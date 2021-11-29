#pragma once

#include <sys/types.h>

#define PATCHED    (1 << 0)
#define OTHER      (1 << 1)
#define INVALID    (1 << 2)
#define NEW_VBMETA "new-vbmeta.img"

int test(const char *image);
int size(const char *image);
int patch(const char *src_img, const char *out_img);
