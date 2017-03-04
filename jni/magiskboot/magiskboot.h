#ifndef _MAGISKBOOT_H_
#define _MAGISKBOOT_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#include "bootimg.h"

#define windowBits 15
#define ZLIB_GZIP 16
#define memLevel 8
#define CHUNK 0x40000

#define LZ4_HEADER_SIZE 19
#define LZ4_FOOTER_SIZE 4

#define CHROMEOS_MAGIC      "CHROMEOS"
#define CHROMEOS_MAGIC_SIZE 8

#define KERNEL_FILE         "kernel"
#define RAMDISK_FILE        "ramdisk.cpio"
#define SECOND_FILE         "second"
#define DTB_FILE            "dtb"
#define NEW_BOOT            "new-boot.img"

#define SUP_LIST "gzip, xz, lzma, lz4, bzip2"
#define SUP_NUM 5

typedef enum {
    UNKNOWN,
    CHROMEOS,
    AOSP,
    ELF,
    GZIP,
    LZOP,
    XZ,
    LZMA,
    BZIP2,
    LZ4,
    MTK,
    QCDT,
} file_t;

extern char *SUP_EXT_LIST[SUP_NUM];
extern file_t SUP_TYPE_LIST[SUP_NUM];
// Cannot declare in header, but place a copy here for convenience
// char *SUP_EXT_LIST[SUP_NUM] = { "gz", "xz", "lzma", "bz2", "lz4" };
// file_t SUP_TYPE_LIST[SUP_NUM] = { GZIP, XZ, LZMA, BZIP2, LZ4 };

// Global variables
extern unsigned char *kernel, *ramdisk, *second, *dtb;
extern boot_img_hdr hdr;
extern file_t boot_type, ramdisk_type, dtb_type;
extern int mtk_kernel, mtk_ramdisk;

// Main entries
void unpack(const char *image);
void repack(const char *image);
void hexpatch(const char *image, const char *from, const char *to);
void cpio(const char *filename);
void error(int rc, const char *msg, ...);
void parse_img(unsigned char *orig, size_t size);
void cleanup();

// Compressions
void gzip(int mode, const char* filename, const unsigned char* buf, size_t size);
void lzma(int mode, const char* filename, const unsigned char* buf, size_t size);
void lz4(int mode, const char* filename, const unsigned char* buf, size_t size);
void bzip2(int mode, const char* filename, const unsigned char* buf, size_t size);
int comp(file_t type, const char *to, const unsigned char *from, size_t size);
void comp_file(const char *method, const char *from);
int decomp(file_t type, const char *to, const unsigned char *from, size_t size);
void decomp_file(char *from);

// Utils
void mmap_ro(const char *filename, unsigned char **buf, size_t *size);
void mmap_rw(const char *filename, unsigned char **buf, size_t *size);
file_t check_type(const unsigned char *buf);
void mem_align(size_t *pos, size_t align);
void file_align(int fd, size_t align);
int open_new(const char *filename);
void print_info();

#endif
