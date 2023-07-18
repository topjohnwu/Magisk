#pragma once

#include <sys/types.h>

#include <base.hpp>

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
int verify(const char *image, const char *cert);
int sign(const char *image, const char *name, const char *cert, const char *key);
int split_image_dtb(const char *filename);
int dtb_commands(int argc, char *argv[]);

static inline bool check_env(const char *name) {
    using namespace std::string_view_literals;
    const char *val = getenv(name);
    return val != nullptr && val == "true"sv;
}
