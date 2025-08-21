#pragma once

#define HEADER_FILE     "header"
#define KERNEL_FILE     "kernel"
#define RAMDISK_FILE    "ramdisk.cpio"
#define VND_RAMDISK_DIR "vendor_ramdisk"
#define SECOND_FILE     "second"
#define EXTRA_FILE      "extra"
#define KER_DTB_FILE    "kernel_dtb"
#define RECV_DTBO_FILE  "recovery_dtbo"
#define DTB_FILE        "dtb"
#define BOOTCONFIG_FILE "bootconfig"
#define NEW_BOOT        "new-boot.img"

int unpack(rust::Utf8CStr image, bool skip_decomp = false, bool hdr = false);
void repack(rust::Utf8CStr src_img, rust::Utf8CStr out_img, bool skip_comp = false);
int split_image_dtb(rust::Utf8CStr filename, bool skip_decomp = false);
void cleanup();
