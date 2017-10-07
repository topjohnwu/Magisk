#ifndef _MAGISKBOOT_H_
#define _MAGISKBOOT_H_

#include <sys/types.h>

#include "bootimg.h"

#define KERNEL_FILE     "kernel"
#define RAMDISK_FILE    "ramdisk.cpio"
#define SECOND_FILE     "second"
#define EXTRA_FILE      "extra"
#define DTB_FILE        "dtb"
#define NEW_BOOT        "new-boot.img"

#define str(a) #a
#define xstr(a) str(a)

// Main entries
void unpack(const char *image);
void repack(const char* orig_image, const char* out_image);
void hexpatch(const char *image, const char *from, const char *to);
int parse_img(void *orig, size_t size, boot_img *boot);
int cpio_commands(const char *command, int argc, char *argv[]);
void comp_file(const char *method, const char *from, const char *to);
void decomp_file(char *from, const char *to);
void dtb_print(const char *file);
void dtb_patch(const char *file);

// Compressions
size_t gzip(int mode, int fd, const void *buf, size_t size);
size_t lzma(int mode, int fd, const void *buf, size_t size);
size_t lz4(int mode, int fd, const void *buf, size_t size);
size_t bzip2(int mode, int fd, const void *buf, size_t size);
size_t lz4_legacy(int mode, int fd, const void *buf, size_t size);
long long comp(file_t type, int to, const void *from, size_t size);
long long decomp(file_t type, int to, const void *from, size_t size);

// Utils
extern void mmap_ro(const char *filename, void **buf, size_t *size);
extern void mmap_rw(const char *filename, void **buf, size_t *size);
extern void write_zero(int fd, size_t size);
extern void mem_align(size_t *pos, size_t align);
extern void file_align(int fd, size_t align, int out);
extern int open_new(const char *filename);
extern void *patch_init_rc(char *data, uint32_t *size);
extern int check_verity_pattern(const char *s);
extern int check_encryption_pattern(const char *s);

#endif
