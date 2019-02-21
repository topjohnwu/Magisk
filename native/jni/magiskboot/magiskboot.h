#ifndef _MAGISKBOOT_H_
#define _MAGISKBOOT_H_

#include <sys/types.h>

#include "format.h"

#define KERNEL_FILE     "kernel"
#define RAMDISK_FILE    "ramdisk.cpio"
#define SECOND_FILE     "second"
#define EXTRA_FILE      "extra"
#define DTB_FILE        "dtb"
#define RECV_DTBO_FILE  "recovery_dtbo"
#define NEW_BOOT        "new-boot.img"

// Main entries
int unpack(const char *image);
void repack(const char* orig_image, const char* out_image);
void hexpatch(const char *image, const char *from, const char *to);
int cpio_commands(int argc, char *argv[]);
int dtb_commands(const char *cmd, int argc, char *argv[]);

// Pattern
int patch_verity(void **buf, uint32_t *size, int patch);
void patch_encryption(void **buf, uint32_t *size);

#endif
