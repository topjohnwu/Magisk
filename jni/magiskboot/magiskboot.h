#ifndef _MAGISKBOOT_H_
#define _MAGISKBOOT_H_

#include <sys/types.h>

#include "logging.h"
#include "bootimg.h"

#define KERNEL_FILE     "kernel"
#define RAMDISK_FILE    "ramdisk.cpio"
#define SECOND_FILE     "second"
#define EXTRA_FILE      "extra"
#define DTB_FILE        "dtb"
#define NEW_BOOT        "new-boot.img"

// Main entries
void unpack(const char *image);
void repack(const char* orig_image, const char* out_image);
void hexpatch(const char *image, const char *from, const char *to);
int parse_img(void *orig, size_t size, boot_img *boot);
int cpio_commands(const char *cmd, int argc, char *argv[]);
void comp_file(const char *method, const char *from, const char *to);
void decomp_file(char *from, const char *to);
int dtb_commands(const char *cmd, int argc, char *argv[]);

// Compressions
size_t gzip(int mode, int fd, const void *buf, size_t size);
size_t lzma(int mode, int fd, const void *buf, size_t size);
size_t lz4(int mode, int fd, const void *buf, size_t size);
size_t bzip2(int mode, int fd, const void *buf, size_t size);
size_t lz4_legacy(int mode, int fd, const void *buf, size_t size);
long long comp(file_t type, int to, const void *from, size_t size);
long long decomp(file_t type, int to, const void *from, size_t size);

#endif
