#pragma once

#include <stdint.h>
#include <utility>
#include "format.hpp"

/****************
 * Other Headers
 ****************/

struct mtk_hdr {
	uint32_t magic;         /* MTK magic */
	uint32_t size;          /* Size of the content */
	char name[32];          /* The type of the header */

	char padding[472];      /* Padding to 512 bytes */
} __attribute__((packed));

struct dhtb_hdr {
	char magic[8];          /* DHTB magic */
	uint8_t checksum[40];   /* Payload SHA256, whole image + SEANDROIDENFORCE + 0xFFFFFFFF */
	uint32_t size;          /* Payload size, whole image + SEANDROIDENFORCE + 0xFFFFFFFF */

	char padding[460];      /* Padding to 512 bytes */
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


/*********************
 * Boot Image Headers
 *********************/

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

// Default to hdr v2
using boot_img_hdr = boot_img_hdr_v2;

// Special Samsung header
struct boot_img_hdr_pxa : public boot_img_hdr_base {
	uint32_t extra_size;   /* extra blob size in bytes */
	uint32_t unknown;
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

#define drct_var(name) \
auto &name() { return img_hdr->name; }
#define decl_var(name, len) \
virtual uint##len##_t &name() { j##len = 0; return j##len; }
#define decl_val(name, type) \
virtual type name() { return 0; }

struct dyn_img_hdr {

	// Direct entries
	drct_var(kernel_size)
	drct_var(ramdisk_size)
	drct_var(second_size)

	// Standard entries
	decl_var(page_size, 32)
	decl_val(header_version, uint32_t)
	decl_var(extra_size, 32)
	decl_var(os_version, 32)
	decl_val(name, char *)
	decl_val(cmdline, char *)
	decl_val(id, char *)
	decl_val(extra_cmdline, char *)

	// v1/v2 specific
	decl_var(recovery_dtbo_size, 32)
	decl_var(recovery_dtbo_offset, 64)
	decl_var(header_size, 32)
	decl_var(dtb_size, 32)

	virtual ~dyn_img_hdr() {
		free(raw);
	}

	virtual size_t hdr_size() = 0;

	boot_img_hdr_base * const &operator* () const {
		return img_hdr;
	}

	void print();
	void dump_hdr_file();
	void load_hdr_file();

protected:
	union {
		/* Main header could be either AOSP or PXA,
		 * but both of them are base headers. */
		boot_img_hdr_base *img_hdr;  /* Common base header */
		boot_img_hdr_v2 *v2_hdr;     /* AOSP v2 header */
		boot_img_hdr_pxa *hdr_pxa;   /* Samsung PXA header */
		void *raw;                   /* Raw pointer */
	};

private:
	// Junk for references
	static uint32_t j32;
	static uint64_t j64;
};

#undef drct_var
#undef decl_var
#undef decl_val

#define __impl_cls(name, hdr) \
protected: name() = default; \
public: \
name(void *ptr) { \
	raw = xmalloc(sizeof(hdr)); \
	memcpy(raw, ptr, sizeof(hdr)); \
} \
size_t hdr_size() override { return sizeof(hdr); }

#define impl_cls(ver) __impl_cls(dyn_img_##ver, boot_img_hdr_##ver)

#define impl_val(name) \
decltype(std::declval<dyn_img_hdr>().name()) name() override { return hdr_pxa->name; }

struct dyn_img_pxa : public dyn_img_hdr {
	impl_cls(pxa)

	impl_val(extra_size)
	impl_val(page_size)
	impl_val(name)
	impl_val(cmdline)
	impl_val(id)
	impl_val(extra_cmdline)
};

#undef impl_val
#define impl_val(name) \
decltype(std::declval<dyn_img_hdr>().name()) name() override { return v2_hdr->name; }

struct dyn_img_v0 : public dyn_img_hdr {
	impl_cls(v0)

	impl_val(page_size)
	impl_val(extra_size)
	impl_val(os_version)
	impl_val(name)
	impl_val(cmdline)
	impl_val(id)
	impl_val(extra_cmdline)
};

struct dyn_img_v1 : public dyn_img_v0 {
	impl_cls(v1)

	impl_val(header_version)
	impl_val(recovery_dtbo_size)
	impl_val(recovery_dtbo_offset)
	impl_val(header_size)

	uint32_t &extra_size() override {
		return dyn_img_hdr::extra_size();
	}
};

struct dyn_img_v2 : public dyn_img_v1 {
	impl_cls(v2)

	impl_val(dtb_size)
};

#undef __impl_cls
#undef impl_cls
#undef impl_val

// Flags
#define MTK_KERNEL      (1 << 0)
#define MTK_RAMDISK     (1 << 1)
#define CHROMEOS_FLAG   (1 << 2)
#define DHTB_FLAG       (1 << 3)
#define SEANDROID_FLAG  (1 << 4)
#define LG_BUMP_FLAG    (1 << 5)
#define SHA256_FLAG     (1 << 6)
#define BLOB_FLAG       (1 << 7)
#define NOOKHD_FLAG     (1 << 8)
#define ACCLAIM_FLAG    (1 << 9)

struct boot_img {
	// Memory map of the whole image
	uint8_t *map_addr;
	size_t map_size;

	// Android image header
	dyn_img_hdr *hdr;

	// Flags to indicate the state of current boot image
	uint16_t flags = 0;

	// The format of kernel and ramdisk
	format_t k_fmt;
	format_t r_fmt;

	/***************************************************
	 * Following pointers points within the mmap region
	 ***************************************************/

	// MTK headers
	mtk_hdr *k_hdr;
	mtk_hdr *r_hdr;

	// Pointer to dtb that is appended after kernel
	uint8_t *kernel_dtb;
	uint32_t kernel_dt_size = 0;

	// Pointer to end of image
	uint8_t *tail;
	size_t tail_size = 0;

	// Pointers to blocks defined in header
	uint8_t *hdr_addr;
	uint8_t *kernel;
	uint8_t *ramdisk;
	uint8_t *second;
	uint8_t *extra;
	uint8_t *recovery_dtbo;
	uint8_t *dtb;

	boot_img(const char *);
	~boot_img();

	void parse_image(uint8_t *addr);
	void find_kernel_dtb();
};
