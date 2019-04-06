#pragma once

#include <sys/types.h>

#define HEADER_FILE     "header"
#define KERNEL_FILE     "kernel"
#define RAMDISK_FILE    "ramdisk.cpio"
#define SECOND_FILE     "second"
#define EXTRA_FILE      "extra"
#define KER_DTB_FILE    "kernel_dtb"
#define RECV_DTBO_FILE  "recovery_dtbo"
#define DTB_FILE        "dtb"
#define NEW_BOOT        "new-boot.img"

// Main entries
int unpack(const char *image, bool hdr = false);
void repack(const char* orig_image, const char* out_image);
void hexpatch(const char *image, const char *from, const char *to);
int cpio_commands(int argc, char *argv[]);
int dtb_commands(const char *cmd, int argc, char *argv[]);

// Pattern
bool patch_verity(void **buf, uint32_t *size, bool patch = true);
void patch_encryption(void **buf, uint32_t *size);
