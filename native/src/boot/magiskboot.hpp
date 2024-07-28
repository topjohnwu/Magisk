#pragma once

#include <sys/types.h>

#include <base.hpp>
#include "bootimg.hpp"
#include "flags.h"

#define HEADER_FILE     "header"
#define KERNEL_FILE     "kernel"
#define RAMDISK_FILE    "ramdisk.cpio"
#define SECOND_FILE     "second"
#define EXTRA_FILE      "extra"
#define KER_DTB_FILE    "kernel_dtb"
#define RECV_DTBO_FILE  "recovery_dtbo"
#define DTB_FILE        "dtb"
#define NEW_BOOT        "new-boot.img"
#define BOOTCONFIG_FILE "bootconfig"
#define VENDOR_RAMDISK_FILE_PREFIX  "vendor_ramdisk_"
#define VENDOR_RAMDISK_FILE_SUFFIX  ".cpio"
#define VENDOR_RAMDISK_FILE         VENDOR_RAMDISK_FILE_PREFIX "%." str(VENDOR_RAMDISK_NAME_SIZE) "s" VENDOR_RAMDISK_FILE_SUFFIX

int unpack(const char *image, bool skip_decomp = false, bool hdr = false, bool accept_vendor = false);
void repack(const char *src_img, const char *out_img, bool skip_comp = false);
int verify(const char *image, const char *cert);
int sign(const char *image, const char *name, const char *cert, const char *key);
int split_image_dtb(const char *filename, bool skip_decomp = false);
int dtb_commands(int argc, char *argv[]);

static inline bool check_env(const char *name) {
    using namespace std::string_view_literals;
    const char *val = getenv(name);
    return val != nullptr && val == "true"sv;
}
