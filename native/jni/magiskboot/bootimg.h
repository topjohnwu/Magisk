#include <stdint.h>
#include "format.h"

#ifndef _BOOT_IMAGE_H_
#define _BOOT_IMAGE_H_

typedef struct boot_img_hdr {
	char magic[8];

	uint32_t kernel_size;  /* size in bytes */
	uint32_t kernel_addr;  /* physical load addr */

	uint32_t ramdisk_size; /* size in bytes */
	uint32_t ramdisk_addr; /* physical load addr */

	uint32_t second_size;  /* size in bytes */
	uint32_t second_addr;  /* physical load addr */

	uint32_t tags_addr;    /* physical addr for kernel tags */
	uint32_t page_size;    /* flash page size we assume */
	uint32_t extra_size;   /* extra blob size in bytes */

	/* operating system version and security patch level; for
	 * version "A.B.C" and patch level "Y-M-D":
	 * ver = A << 14 | B << 7 | C         (7 bits for each of A, B, C)
	 * lvl = ((Y - 2000) & 127) << 4 | M  (7 bits for Y, 4 bits for M)
	 * os_version = ver << 11 | lvl */
	uint32_t os_version;

	char name[16];         /* asciiz product name */

	char cmdline[512];

	uint32_t id[8];        /* timestamp / checksum / sha1 / etc */

	/* Supplemental command line data; kept here to maintain
	 * binary compatibility with older versions of mkbootimg */
	char extra_cmdline[1024];
} __attribute__((packed)) boot_img_hdr ;

typedef struct pxa_boot_img_hdr {
	char magic[8];

	uint32_t kernel_size;  /* size in bytes */
	uint32_t kernel_addr;  /* physical load addr */

	uint32_t ramdisk_size; /* size in bytes */
	uint32_t ramdisk_addr; /* physical load addr */

	uint32_t second_size;  /* size in bytes */
	uint32_t second_addr;  /* physical load addr */

	uint32_t extra_size;   /* extra blob size in bytes */
	uint32_t unknown;      /* unknown value */
	uint32_t tags_addr;    /* physical addr for kernel tags */
	uint32_t page_size;    /* flash page size we assume */

	char name[24];         /* asciiz product name */

	char cmdline[512];

	uint32_t id[8];        /* timestamp / checksum / sha1 / etc */

	/* Supplemental command line data; kept here to maintain
	 * binary compatibility with older versions of mkbootimg */
	char extra_cmdline[1024];
} __attribute__((packed)) pxa_boot_img_hdr;

/*
** +-----------------+
** | boot header     | 1 page
** +-----------------+
** | kernel          | n pages
** +-----------------+
** | ramdisk         | m pages
** +-----------------+
** | second stage    | o pages
** +-----------------+
** | extra blob      | p pages
** +-----------------+
**
** n = (kernel_size + page_size - 1) / page_size
** m = (ramdisk_size + page_size - 1) / page_size
** o = (second_size + page_size - 1) / page_size
** p = (extra_size + page_size - 1) / page_size
**
** 0. all entities are page_size aligned in flash
** 1. kernel and ramdisk are required (size != 0)
** 2. second is optional (second_size == 0 -> no second)
** 3. load each element (kernel, ramdisk, second) at
**    the specified physical address (kernel_addr, etc)
** 4. prepare tags at tag_addr.  kernel_args[] is
**    appended to the kernel commandline in the tags.
** 5. r0 = 0, r1 = MACHINE_TYPE, r2 = tags_addr
** 6. if second_size != 0: jump to second_addr
**    else: jump to kernel_addr
*/

typedef struct mtk_hdr {
	uint32_t magic;      /* MTK magic */
	uint32_t size;       /* Size of the content */
	char name[32];       /* The type of the header */
} __attribute__((packed)) mtk_hdr;

// Flags
#define MTK_KERNEL    0x01
#define MTK_RAMDISK   0x02
#define CHROMEOS_FLAG 0x04
#define PXA_FLAG      0x08

typedef struct boot_img {
	// Memory map of the whole image
	void *map_addr;
	size_t map_size;

	// Headers
	void *hdr;  		/* Either boot_img_hdr or pxa_boot_img_hdr */
	mtk_hdr *k_hdr;		/* MTK kernel header */
	mtk_hdr *r_hdr;		/* MTK ramdisk header */

	// Flags to indicate the state of current boot image
	uint8_t flags;

	// The format of kernel and ramdisk
	format_t k_fmt;
	format_t r_fmt;

	// Pointer to dtb that is appended after kernel
	void *dtb;
	uint32_t dt_size;

	// Pointer to end of image
	void *tail;
	size_t tail_size;

	// Pointers to blocks defined in header
	void *kernel;
	void *ramdisk;
	void *second;
	void *extra;
} boot_img;

#endif
