#pragma once

#include <stdint.h>
#include "format.h"

struct boot_img_hdr_base {
	char magic[8];

	uint32_t kernel_size;  /* size in bytes */
	uint32_t kernel_addr;  /* physical load addr */

	uint32_t ramdisk_size; /* size in bytes */
	uint32_t ramdisk_addr; /* physical load addr */

	uint32_t second_size;  /* size in bytes */
	uint32_t second_addr;  /* physical load addr */
} __attribute__((packed));

struct boot_img_hdr_v0 : public boot_img_hdr_base {
	uint32_t tags_addr;    /* physical addr for kernel tags */
	uint32_t page_size;    /* flash page size we assume */

	/* In header v1, this field is used for header version
	 * However, on some devices like Samsung, this field is used to store DTB
	 * We will treat this field differently based on its value */
	union {
		uint32_t header_version;  /* the version of the header */
		uint32_t extra_size;      /* extra blob size in bytes */
	};

	/* operating system version and security patch level; for
	 * version "A.B.C" and patch level "Y-M-D":
	 * ver = A << 14 | B << 7 | C         (7 bits for each of A, B, C)
	 * lvl = ((Y - 2000) & 127) << 4 | M  (7 bits for Y, 4 bits for M)
	 * os_version = ver << 11 | lvl */
	uint32_t os_version;

	char name[16];         /* asciiz product name */
	char cmdline[512];
	char id[32];           /* timestamp / checksum / sha1 / etc */

	/* Supplemental command line data; kept here to maintain
	 * binary compatibility with older versions of mkbootimg */
	char extra_cmdline[1024];
} __attribute__((packed));

struct boot_img_hdr_v1 : public boot_img_hdr_v0 {
	uint32_t recovery_dtbo_size;    /* size in bytes for recovery DTBO/ACPIO image */
	uint64_t recovery_dtbo_offset;  /* offset to recovery dtbo/acpio in boot image */
	uint32_t header_size;
} __attribute__((packed));

struct boot_img_hdr_v2 : public boot_img_hdr_v1 {
	uint32_t dtb_size;  /* size in bytes for DTB image */
	uint64_t dtb_addr;  /* physical load address for DTB image */
} __attribute__((packed));

// Default to hdr v1
typedef boot_img_hdr_v2 boot_img_hdr;

// Special Samsung header
struct boot_img_hdr_pxa : public boot_img_hdr_base {
	uint32_t extra_size;   /* extra blob size in bytes */
	uint32_t unknown;      /* unknown value */
	uint32_t tags_addr;    /* physical addr for kernel tags */
	uint32_t page_size;    /* flash page size we assume */

	char name[24];         /* asciiz product name */
	char cmdline[512];
	char id[32];           /* timestamp / checksum / sha1 / etc */

	/* Supplemental command line data; kept here to maintain
	 * binary compatibility with older versions of mkbootimg */
	char extra_cmdline[1024];
} __attribute__((packed));

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
** | recovery dtbo   | q pages
** +-----------------+
** | dtb             | r pages
** +-----------------+
**
** n = (kernel_size + page_size - 1) / page_size
** m = (ramdisk_size + page_size - 1) / page_size
** o = (second_size + page_size - 1) / page_size
** p = (extra_size + page_size - 1) / page_size
** q = (recovery_dtbo_size + page_size - 1) / page_size
** r = (dtb_size + page_size - 1) / page_size
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

struct mtk_hdr {
	uint32_t magic;         /* MTK magic */
	uint32_t size;          /* Size of the content */
	char name[32];          /* The type of the header */
} __attribute__((packed));

struct dhtb_hdr {
	char magic[8];          /* DHTB magic */
	uint8_t checksum[40];   /* Payload SHA256, whole image + SEANDROIDENFORCE + 0xFFFFFFFF */
	uint32_t size;          /* Payload size, whole image + SEANDROIDENFORCE + 0xFFFFFFFF */
} __attribute__((packed));

struct blob_hdr {
	char secure_magic[20];  /* "-SIGNED-BY-SIGNBLOB-" */
	uint32_t datalen;       /* 0x00000000 */
	uint32_t signature;     /* 0x00000000 */
	char magic[16];         /* "MSM-RADIO-UPDATE" */
	uint32_t hdr_version;   /* 0x00010000 */
	uint32_t hdr_size;      /* Size of header */
	uint32_t part_offset;   /* Same as size */
	uint32_t num_parts;     /* Number of partitions */
	uint32_t unknown[7];    /* All 0x00000000 */
	char name[4];           /* Name of partition */
	uint32_t offset;        /* offset in blob where this partition starts */
	uint32_t size;          /* Size of data */
	uint32_t version;       /* 0x00000001 */
} __attribute__((packed));

// Flags
#define MTK_KERNEL      1 << 1
#define MTK_RAMDISK     1 << 2
#define CHROMEOS_FLAG   1 << 3
#define DHTB_FLAG       1 << 4
#define SEANDROID_FLAG  1 << 5
#define LG_BUMP_FLAG    1 << 6
#define SHA256_FLAG     1 << 7
#define BLOB_FLAG       1 << 8
#define NOOKHD_FLAG     1 << 9
#define ACCLAIM_FLAG    1 << 10

struct dyn_img_hdr {

#define dyn_access(x) (pxa ? hdr_pxa->x : v2_hdr->x)

#define dyn_get(name, type) \
type name() const { return dyn_access(name); }
#define dyn_ref(name, type) \
type &name() { return dyn_access(name); }
#define v2_ref(name, type, alt) \
type &name() { if (pxa) { alt = 0; return alt; } return v2_hdr->name; }

	dyn_ref(page_size, uint32_t);
	dyn_get(name, char *);
	dyn_get(cmdline, char *);
	dyn_get(id, char *);
	dyn_get(extra_cmdline, char *);

	v2_ref(os_version, uint32_t, j32);
	v2_ref(recovery_dtbo_size, uint32_t, j32);
	v2_ref(recovery_dtbo_offset, uint64_t, j64);
	v2_ref(header_size, uint32_t, j32);
	v2_ref(dtb_size, uint32_t, j32);

	dyn_img_hdr() : pxa(false), img_hdr(nullptr) {}
	~dyn_img_hdr() {
		if (pxa)
			delete hdr_pxa;
		else
			delete v2_hdr;
	}

	uint32_t header_version() {
		// There won't be v4 header any time soon...
		// If larger than 4, assume this field will be treated as extra_size
		return pxa || v2_hdr->header_version > 4 ? 0 : v2_hdr->header_version;
	}

	uint32_t &extra_size() {
		// If header version > 0, we should treat this field as header_version
		if (header_version()) {
			j32 = 0;
			return j32;
		}
		return dyn_access(extra_size);
	}

	size_t hdr_size() {
		return pxa ? sizeof(boot_img_hdr_pxa) : sizeof(boot_img_hdr);
	}

	void set_hdr(boot_img_hdr *h) {
		v2_hdr = h;
	}

	void set_hdr(boot_img_hdr_pxa *h) {
		hdr_pxa = h;
		pxa = true;
	}

	boot_img_hdr_base *operator-> () const {
		return img_hdr;
	};

	boot_img_hdr_base * const &operator* () const {
		return img_hdr;
	}

private:
	bool pxa;
	union {
		/* Main header could be either AOSP or PXA,
		 * but both of them is a base header.
		 * Same address can be interpreted in 3 ways */
		boot_img_hdr_base *img_hdr;  /* Common base header */
		boot_img_hdr *v2_hdr;        /* AOSP v2 header */
		boot_img_hdr_pxa *hdr_pxa;   /* Samsung PXA header */
	};

	static uint32_t j32;
	static uint64_t j64;
};

struct boot_img {
	// Memory map of the whole image
	uint8_t *map_addr;
	size_t map_size;

	// Headers
	dyn_img_hdr hdr;    /* Android image header */
	mtk_hdr *k_hdr;     /* MTK kernel header */
	mtk_hdr *r_hdr;     /* MTK ramdisk header */
	blob_hdr *b_hdr;    /* Tegra blob header */

	// Flags to indicate the state of current boot image
	uint16_t flags;

	// The format of kernel and ramdisk
	format_t k_fmt;
	format_t r_fmt;

	// Pointer to dtb that is appended after kernel
	uint8_t *kernel_dtb;
	uint32_t kernel_dt_size;

	// Pointer to end of image
	uint8_t *tail;
	size_t tail_size;

	// Pointers to blocks defined in header
	uint8_t *kernel;
	uint8_t *ramdisk;
	uint8_t *second;
	uint8_t *extra;
	uint8_t *recov_dtbo;
	uint8_t *dtb;

	~boot_img();

	int parse_file(const char *);
	int parse_image(uint8_t *);
	void find_dtb();
	void print_hdr();
};
