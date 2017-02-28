#ifndef _ARCHIVE_H_
#define _ARCHIVE_H_

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

#define CHROMEOS_MAGIC      "CHROMEOS"
#define CHROMEOS_MAGIC_SIZE 8

#define KERNEL_FILE         "kernel"
#define RAMDISK_FILE        "ramdisk.cpio"
#define SECOND_FILE         "second"
#define DTB_FILE            "dtb"

typedef enum {
    DONTCARE,
    CHROMEOS,
    AOSP,
    ELF,
    GZIP,
    LZOP,
    XZ,
    LZMA,
    BZIP2,
    LZ4,
    QCDT,
} file_t;

// Global variables
extern unsigned char *base, *kernel, *ramdisk, *second, *dtb;
extern boot_img_hdr hdr;
extern file_t boot_type, ramdisk_type, dtb_type;
extern int mtk_kernel, mtk_ramdisk;

// Main entries
void unpack(const char *image);
void repack(const char *image);
void hexpatch(char *image, char *from, char *to);
void error(int rc, const char *msg, ...);

// Parse image
void parse_img(unsigned char *orig, size_t size);

// Compressions
void gzip(int mode, const char* filename, unsigned char* buf, size_t size);
void lzma(int mode, const char* filename, unsigned char* buf, size_t size);

#endif
