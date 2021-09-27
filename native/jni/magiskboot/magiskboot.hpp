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

int unpack(const char *image, bool skip_decomp = false, bool hdr = false);
void repack(const char *src_img, const char *out_img, bool skip_comp = false);
int split_image_dtb(const char *filename);
int hexpatch(const char *image, const char *from, const char *to);
int cpio_commands(int argc, char *argv[]);
int dtb_commands(int argc, char *argv[]);

uint32_t patch_verity(void *buf, uint32_t size);
uint32_t patch_encryption(void *buf, uint32_t size);
bool check_env(const char *name);
