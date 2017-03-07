#ifndef _MAGISKBOOT_H_
#define _MAGISKBOOT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
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

typedef enum {
    NONE,
    RM,
    MKDIR,
    ADD
} command_t;

extern char *SUP_EXT_LIST[SUP_NUM];
extern file_t SUP_TYPE_LIST[SUP_NUM];
// Cannot declare in header, but place a copy here for convenience
// char *SUP_EXT_LIST[SUP_NUM] = { "gz", "xz", "lzma", "bz2", "lz4" };
// file_t SUP_TYPE_LIST[SUP_NUM] = { GZIP, XZ, LZMA, BZIP2, LZ4 };

// Vector
typedef struct vector {
    size_t size;
    size_t cap;
    void **data;
} vector;
void vec_init(vector *v);
void vec_push_back(vector *v, void *p);
void vec_destroy(vector *v);

#define vec_size(v) (v)->size
#define vec_cap(v) (v)->cap
#define vec_entry(v) (v)->data
// vec_for_each(vector *v, void *e)
#define vec_for_each(v, e) \
    e = (v)->data[0]; \
    for (size_t _i = 0; _i < (v)->size; ++_i, e = (v)->data[_i])

// Global variables
extern unsigned char *kernel, *ramdisk, *second, *dtb;
extern boot_img_hdr hdr;
extern file_t boot_type, ramdisk_type, dtb_type;
extern int mtk_kernel, mtk_ramdisk;

// Main entries
void unpack(const char *image);
void repack(const char* orig_image, const char* out_image);
void hexpatch(const char *image, const char *from, const char *to);
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

// CPIO
void parse_cpio(const char *filename, vector *v);
void dump_cpio(const char *filename, vector *v);
void cpio_vec_destroy(vector *v);
void cpio_rm(int recursive, const char *entry, vector *v);
void cpio_mkdir(mode_t mode, const char *entry, vector *v);
void cpio_add(mode_t mode, const char *entry, const char *filename, vector *v);

// Utils
void mmap_ro(const char *filename, unsigned char **buf, size_t *size);
void mmap_rw(const char *filename, unsigned char **buf, size_t *size);
file_t check_type(const unsigned char *buf);
void mem_align(size_t *pos, size_t align);
void file_align(int fd, size_t align, int out);
int open_new(const char *filename);
void print_info();

#endif
