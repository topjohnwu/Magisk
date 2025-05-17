#pragma once

#include <cstdint>
#include <utility>
#include <bitset>
#include <cxx.h>

#include "format.hpp"

/******************
 * Special Headers
 *****************/

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

struct zimage_hdr {
    uint32_t code[9];
    uint32_t magic;      /* zImage magic */
    uint32_t start;      /* absolute load/run zImage address */
    uint32_t end;        /* zImage end address */
    uint32_t endian;     /* endianness flag */
    // There could be more fields, but we don't care
} __attribute__((packed));

/**************
 * AVB Headers
 **************/

#define AVB_FOOTER_MAGIC_LEN 4
#define AVB_MAGIC_LEN 4
#define AVB_RELEASE_STRING_SIZE 48

// https://android.googlesource.com/platform/external/avb/+/refs/heads/android11-release/libavb/avb_footer.h
struct AvbFooter {
    uint8_t magic[AVB_FOOTER_MAGIC_LEN];
    uint32_t version_major;
    uint32_t version_minor;
    uint64_t original_image_size;
    uint64_t vbmeta_offset;
    uint64_t vbmeta_size;
    uint8_t reserved[28];
} __attribute__((packed));

// https://android.googlesource.com/platform/external/avb/+/refs/heads/android11-release/libavb/avb_vbmeta_image.h
struct AvbVBMetaImageHeader {
    uint8_t magic[AVB_MAGIC_LEN];
    uint32_t required_libavb_version_major;
    uint32_t required_libavb_version_minor;
    uint64_t authentication_data_block_size;
    uint64_t auxiliary_data_block_size;
    uint32_t algorithm_type;
    uint64_t hash_offset;
    uint64_t hash_size;
    uint64_t signature_offset;
    uint64_t signature_size;
    uint64_t public_key_offset;
    uint64_t public_key_size;
    uint64_t public_key_metadata_offset;
    uint64_t public_key_metadata_size;
    uint64_t descriptors_offset;
    uint64_t descriptors_size;
    uint64_t rollback_index;
    uint32_t flags;
    uint32_t rollback_index_location;
    uint8_t release_string[AVB_RELEASE_STRING_SIZE];
    uint8_t reserved[80];
} __attribute__((packed));

/*********************
 * Boot Image Headers
 *********************/

// https://android.googlesource.com/platform/system/tools/mkbootimg/+/refs/heads/android12-release/include/bootimg/bootimg.h

#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ID_SIZE 32
#define BOOT_ARGS_SIZE 512
#define BOOT_EXTRA_ARGS_SIZE 1024
#define VENDOR_BOOT_ARGS_SIZE 2048
#define VENDOR_RAMDISK_NAME_SIZE 32
#define VENDOR_RAMDISK_TABLE_ENTRY_BOARD_ID_SIZE 16

#define VENDOR_RAMDISK_TYPE_NONE 0
#define VENDOR_RAMDISK_TYPE_PLATFORM 1
#define VENDOR_RAMDISK_TYPE_RECOVERY 2
#define VENDOR_RAMDISK_TYPE_DLKM 3

/*
 * When the boot image header has a version of 0 - 2, the structure of the boot
 * image is as follows:
 *
 * +-----------------+
 * | boot header     | 1 page
 * +-----------------+
 * | kernel          | m pages
 * +-----------------+
 * | ramdisk         | n pages
 * +-----------------+
 * | second stage    | o pages
 * +-----------------+
 * | extra blob      | x pages (non standard)
 * +-----------------+
 * | recovery dtbo   | p pages
 * +-----------------+
 * | dtb             | q pages
 * +-----------------+
 *
 * m = (kernel_size + page_size - 1) / page_size
 * n = (ramdisk_size + page_size - 1) / page_size
 * o = (second_size + page_size - 1) / page_size
 * p = (recovery_dtbo_size + page_size - 1) / page_size
 * q = (dtb_size + page_size - 1) / page_size
 * x = (extra_size + page_size - 1) / page_size
 */

struct boot_img_hdr_v0_common {
    char magic[BOOT_MAGIC_SIZE];

    uint32_t kernel_size;  /* size in bytes */
    uint32_t kernel_addr;  /* physical load addr */

    uint32_t ramdisk_size; /* size in bytes */
    uint32_t ramdisk_addr; /* physical load addr */

    uint32_t second_size;  /* size in bytes */
    uint32_t second_addr;  /* physical load addr */
} __attribute__((packed));

struct boot_img_hdr_v0 : public boot_img_hdr_v0_common {
    uint32_t tags_addr;    /* physical addr for kernel tags */

    // In AOSP headers, this field is used for page size.
    // For Samsung PXA headers, the use of this field is unknown;
    // however, its value is something unrealistic to be treated as page size.
    // We use this fact to determine whether this is an AOSP or PXA header.
    union {
        uint32_t unknown;
        uint32_t page_size;    /* flash page size we assume */
    };

    // In header v1, this field is used for header version
    // However, on some devices like Samsung, this field is used to store DTB
    // We treat this field differently based on its value
    union {
        uint32_t header_version;  /* the version of the header */
        uint32_t extra_size;      /* extra blob size in bytes */
    };

    // Operating system version and security patch level.
    // For version "A.B.C" and patch level "Y-M-D":
    //   (7 bits for each of A, B, C; 7 bits for (Y-2000), 4 bits for M)
    //   os_version = A[31:25] B[24:18] C[17:11] (Y-2000)[10:4] M[3:0]
    uint32_t os_version;

    char name[BOOT_NAME_SIZE];  /* asciiz product name */
    char cmdline[BOOT_ARGS_SIZE];
    char id[BOOT_ID_SIZE];      /* timestamp / checksum / sha1 / etc */

    // Supplemental command line data; kept here to maintain
    // binary compatibility with older versions of mkbootimg.
    char extra_cmdline[BOOT_EXTRA_ARGS_SIZE];
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

// Special Samsung header
struct boot_img_hdr_pxa : public boot_img_hdr_v0_common {
    uint32_t extra_size;   /* extra blob size in bytes */
    uint32_t unknown;
    uint32_t tags_addr;    /* physical addr for kernel tags */
    uint32_t page_size;    /* flash page size we assume */

    char name[24];         /* asciiz product name */
    char cmdline[BOOT_ARGS_SIZE];
    char id[BOOT_ID_SIZE]; /* timestamp / checksum / sha1 / etc */

    char extra_cmdline[BOOT_EXTRA_ARGS_SIZE];
} __attribute__((packed));

/*
 * When the boot image header has a version of 3 - 4, the structure of the boot
 * image is as follows:
 *
 * +---------------------+
 * | boot header         | 4096 bytes
 * +---------------------+
 * | kernel              | m pages
 * +---------------------+
 * | ramdisk             | n pages
 * +---------------------+
 * | boot signature      | g pages
 * +---------------------+
 *
 * m = (kernel_size + 4096 - 1) / 4096
 * n = (ramdisk_size + 4096 - 1) / 4096
 * g = (signature_size + 4096 - 1) / 4096
 *
 * Page size is fixed at 4096 bytes.
 *
 * The structure of the vendor boot image is as follows:
 *
 * +------------------------+
 * | vendor boot header     | o pages
 * +------------------------+
 * | vendor ramdisk section | p pages
 * +------------------------+
 * | dtb                    | q pages
 * +------------------------+
 * | vendor ramdisk table   | r pages
 * +------------------------+
 * | bootconfig             | s pages
 * +------------------------+
 *
 * o = (2128 + page_size - 1) / page_size
 * p = (vendor_ramdisk_size + page_size - 1) / page_size
 * q = (dtb_size + page_size - 1) / page_size
 * r = (vendor_ramdisk_table_size + page_size - 1) / page_size
 * s = (vendor_bootconfig_size + page_size - 1) / page_size
 *
 * Note that in version 4 of the vendor boot image, multiple vendor ramdisks can
 * be included in the vendor boot image. The bootloader can select a subset of
 * ramdisks to load at runtime. To help the bootloader select the ramdisks, each
 * ramdisk is tagged with a type tag and a set of hardware identifiers
 * describing the board, soc or platform that this ramdisk is intended for.
 *
 * The vendor ramdisk section is consist of multiple ramdisk images concatenated
 * one after another, and vendor_ramdisk_size is the size of the section, which
 * is the total size of all the ramdisks included in the vendor boot image.
 *
 * The vendor ramdisk table holds the size, offset, type, name and hardware
 * identifiers of each ramdisk. The type field denotes the type of its content.
 * The vendor ramdisk names are unique. The hardware identifiers are specified
 * in the board_id field in each table entry. The board_id field is consist of a
 * vector of unsigned integer words, and the encoding scheme is defined by the
 * hardware vendor.
 *
 * For the different type of ramdisks, there are:
 *    - VENDOR_RAMDISK_TYPE_NONE indicates the value is unspecified.
 *    - VENDOR_RAMDISK_TYPE_PLATFORM ramdisks contain platform specific bits, so
 *      the bootloader should always load these into memory.
 *    - VENDOR_RAMDISK_TYPE_RECOVERY ramdisks contain recovery resources, so
 *      the bootloader should load these when booting into recovery.
 *    - VENDOR_RAMDISK_TYPE_DLKM ramdisks contain dynamic loadable kernel
 *      modules.
 *
 * Version 4 of the vendor boot image also adds a bootconfig section to the end
 * of the image. This section contains Boot Configuration parameters known at
 * build time. The bootloader is responsible for placing this section directly
 * after the generic ramdisk, followed by the bootconfig trailer, before
 * entering the kernel.
 */

struct boot_img_hdr_v3 {
    uint8_t magic[BOOT_MAGIC_SIZE];

    uint32_t kernel_size;  /* size in bytes */
    uint32_t ramdisk_size; /* size in bytes */
    uint32_t os_version;
    uint32_t header_size;
    uint32_t reserved[4];

    uint32_t header_version;

    char cmdline[BOOT_ARGS_SIZE + BOOT_EXTRA_ARGS_SIZE];
} __attribute__((packed));

struct boot_img_hdr_vnd_v3 {
    // Must be VENDOR_BOOT_MAGIC.
    uint8_t magic[BOOT_MAGIC_SIZE];
    // Version of the vendor boot image header.
    uint32_t header_version;
    uint32_t page_size;     /* flash page size we assume */
    uint32_t kernel_addr;   /* physical load addr */
    uint32_t ramdisk_addr;  /* physical load addr */
    uint32_t ramdisk_size;  /* size in bytes */
    char cmdline[VENDOR_BOOT_ARGS_SIZE];
    uint32_t tags_addr;     /* physical addr for kernel tags (if required) */
    char name[BOOT_NAME_SIZE]; /* asciiz product name */
    uint32_t header_size;
    uint32_t dtb_size;      /* size in bytes for DTB image */
    uint64_t dtb_addr;      /* physical load address for DTB image */
} __attribute__((packed));

struct boot_img_hdr_v4 : public boot_img_hdr_v3 {
    uint32_t signature_size; /* size in bytes */
} __attribute__((packed));

struct boot_img_hdr_vnd_v4 : public boot_img_hdr_vnd_v3 {
    uint32_t vendor_ramdisk_table_size;       /* size in bytes for the vendor ramdisk table */
    uint32_t vendor_ramdisk_table_entry_num;  /* number of entries in the vendor ramdisk table */
    uint32_t vendor_ramdisk_table_entry_size; /* size in bytes for a vendor ramdisk table entry */
    uint32_t bootconfig_size; /* size in bytes for the bootconfig section */
} __attribute__((packed));

struct vendor_ramdisk_table_entry_v4 {
    uint32_t ramdisk_size;   /* size in bytes for the ramdisk image */
    uint32_t ramdisk_offset; /* offset to the ramdisk image in vendor ramdisk section */
    uint32_t ramdisk_type;   /* type of the ramdisk */
    char ramdisk_name[VENDOR_RAMDISK_NAME_SIZE]; /* asciiz ramdisk name */

    // Hardware identifiers describing the board, soc or platform which this
    // ramdisk is intended to be loaded on.
    uint32_t board_id[VENDOR_RAMDISK_TABLE_ENTRY_BOARD_ID_SIZE];
} __attribute__((packed));

/*******************************
 * Polymorphic Universal Header
 *******************************/

#define decl_val(name, len) \
virtual uint##len##_t name() const { return 0; }

#define decl_var(name, len) \
virtual uint##len##_t &name() { return j##len(); } \
decl_val(name, len)

#define decl_str(name) \
virtual char *name() { return nullptr; } \
virtual const char *name() const { return nullptr; }

struct dyn_img_hdr {

    virtual bool is_vendor() const = 0;

    // Standard entries
    decl_var(kernel_size, 32)
    decl_var(ramdisk_size, 32)
    decl_var(second_size, 32)
    decl_val(page_size, 32)
    decl_val(header_version, 32)
    decl_var(extra_size, 32)
    decl_var(os_version, 32)
    decl_str(name)
    decl_str(cmdline)
    decl_str(id)
    decl_str(extra_cmdline)

    // v1/v2 specific
    decl_var(recovery_dtbo_size, 32)
    decl_var(recovery_dtbo_offset, 64)
    decl_var(header_size, 32)
    decl_var(dtb_size, 32)

    // v4 specific
    decl_val(signature_size, 32)

    // v4 vendor specific
    decl_val(vendor_ramdisk_table_size, 32)
    decl_val(vendor_ramdisk_table_entry_num, 32)
    decl_val(vendor_ramdisk_table_entry_size, 32)
    decl_var(bootconfig_size, 32)

    virtual ~dyn_img_hdr() {
        free(raw);
    }

    virtual size_t hdr_size() const = 0;
    virtual size_t hdr_space() const { return page_size(); }
    virtual dyn_img_hdr *clone() const = 0;

    const void *raw_hdr() const { return raw; }
    void print() const;
    void dump_hdr_file() const;
    void load_hdr_file();

protected:
    union {
        boot_img_hdr_v2 *v2_hdr;     /* AOSP v2 header */
        boot_img_hdr_v4 *v4_hdr;     /* AOSP v4 header */
        boot_img_hdr_vnd_v4 *v4_vnd; /* AOSP vendor v4 header */
        boot_img_hdr_pxa *hdr_pxa;   /* Samsung PXA header */
        void *raw;                   /* Raw pointer */
    };

    static uint32_t &j32() { _j32 = 0; return _j32; }
    static uint64_t &j64() { _j64 = 0; return _j64; }

private:
    // Junk for references
    inline static uint32_t _j32 = 0;
    inline static uint64_t _j64 = 0;
};

#undef decl_var
#undef decl_val
#undef decl_str

#define __impl_cls(name, hdr)           \
protected: name() = default;            \
public:                                 \
name(const void *ptr) {                 \
    raw = malloc(sizeof(hdr));          \
    memcpy(raw, ptr, sizeof(hdr));      \
}                                       \
size_t hdr_size() const override {      \
    return sizeof(hdr);                 \
}                                       \
dyn_img_hdr *clone() const override {   \
    auto p = new name(raw);             \
    return p;                           \
};

#define __impl_val(name, hdr_name) \
decltype(std::declval<const dyn_img_hdr>().name()) name() const override { return hdr_name->name; }

#define __impl_var(name, hdr_name) \
decltype(std::declval<dyn_img_hdr>().name()) name() override { return hdr_name->name; } \
__impl_val(name, hdr_name)

#define impl_cls(ver)  __impl_cls(dyn_img_##ver, boot_img_hdr_##ver)
#define impl_val(name) __impl_val(name, v2_hdr)
#define impl_var(name) __impl_var(name, v2_hdr)

struct dyn_img_hdr_boot : public dyn_img_hdr {
    bool is_vendor() const final { return false; }
};

struct dyn_img_common : public dyn_img_hdr_boot {
    impl_var(kernel_size)
    impl_var(ramdisk_size)
    impl_var(second_size)
};

struct dyn_img_v0 : public dyn_img_common {
    impl_cls(v0)

    impl_val(page_size)
    impl_var(extra_size)
    impl_var(os_version)
    impl_var(name)
    impl_var(cmdline)
    impl_var(id)
    impl_var(extra_cmdline)
};

struct dyn_img_v1 : public dyn_img_v0 {
    impl_cls(v1)

    impl_val(header_version)
    impl_var(recovery_dtbo_size)
    impl_var(recovery_dtbo_offset)
    impl_var(header_size)

    uint32_t &extra_size() override { return j32(); }
    uint32_t extra_size() const override { return 0; }
};

struct dyn_img_v2 : public dyn_img_v1 {
    impl_cls(v2)

    impl_var(dtb_size)
};

#undef impl_val
#undef impl_var
#define impl_val(name) __impl_val(name, hdr_pxa)
#define impl_var(name) __impl_var(name, hdr_pxa)

struct dyn_img_pxa : public dyn_img_common {
    impl_cls(pxa)

    impl_var(extra_size)
    impl_val(page_size)
    impl_var(name)
    impl_var(cmdline)
    impl_var(id)
    impl_var(extra_cmdline)
};

#undef impl_val
#undef impl_var
#define impl_val(name) __impl_val(name, v4_hdr)
#define impl_var(name) __impl_var(name, v4_hdr)

struct dyn_img_v3 : public dyn_img_hdr_boot {
    impl_cls(v3)

    impl_var(kernel_size)
    impl_var(ramdisk_size)
    impl_var(os_version)
    impl_var(header_size)
    impl_val(header_version)
    impl_var(cmdline)

    // Make API compatible
    uint32_t page_size() const override { return 4096; }
    char *extra_cmdline() override { return &v4_hdr->cmdline[BOOT_ARGS_SIZE]; }
    const char *extra_cmdline() const override { return &v4_hdr->cmdline[BOOT_ARGS_SIZE]; }
};

struct dyn_img_v4 : public dyn_img_v3 {
    impl_cls(v4)

    impl_val(signature_size)
};

struct dyn_img_hdr_vendor : public dyn_img_hdr {
    bool is_vendor() const final { return true; }
};

#undef impl_val
#undef impl_var
#define impl_val(name) __impl_val(name, v4_vnd)
#define impl_var(name) __impl_var(name, v4_vnd)

struct dyn_img_vnd_v3 : public dyn_img_hdr_vendor {
    impl_cls(vnd_v3)

    impl_val(header_version)
    impl_val(page_size)
    impl_var(ramdisk_size)
    impl_var(cmdline)
    impl_var(name)
    impl_var(header_size)
    impl_var(dtb_size)

    size_t hdr_space() const override { return align_to(hdr_size(), page_size()); }

    // Make API compatible
    char *extra_cmdline() override { return &v4_vnd->cmdline[BOOT_ARGS_SIZE]; }
    const char *extra_cmdline() const override { return &v4_vnd->cmdline[BOOT_ARGS_SIZE]; }
};

struct dyn_img_vnd_v4 : public dyn_img_vnd_v3 {
    impl_cls(vnd_v4)

    impl_val(vendor_ramdisk_table_size)
    impl_val(vendor_ramdisk_table_entry_num)
    impl_val(vendor_ramdisk_table_entry_size)
    impl_var(bootconfig_size)
};

#undef __impl_cls
#undef __impl_val
#undef __impl_var
#undef impl_cls
#undef impl_val
#undef impl_var

/******************
 * Full Boot Image
 ******************/

enum {
    MTK_KERNEL,
    MTK_RAMDISK,
    CHROMEOS_FLAG,
    DHTB_FLAG,
    SEANDROID_FLAG,
    LG_BUMP_FLAG,
    SHA256_FLAG,
    BLOB_FLAG,
    NOOKHD_FLAG,
    ACCLAIM_FLAG,
    AMONET_FLAG,
    AVB1_SIGNED_FLAG,
    AVB_FLAG,
    ZIMAGE_KERNEL,
    BOOT_FLAGS_MAX
};

struct boot_img {
    // Memory map of the whole image
    const mmap_data map;

    // Android image header
    const dyn_img_hdr *hdr = nullptr;

    // Flags to indicate the state of current boot image
    std::bitset<BOOT_FLAGS_MAX> flags;

    // The format of kernel, ramdisk and extra
    FileFormat k_fmt;
    FileFormat r_fmt;
    FileFormat e_fmt;

    /*************************************************************
     * Following pointers points within the read-only mmap region
     *************************************************************/

    // Layout of the memory mapped region
    // +---------+
    // | head    | Vendor specific. Should not exist for standard AOSP boot images.
    // +---------+
    // | payload | The actual entire AOSP boot image, including the boot image header.
    // +---------+
    // | tail    | Data after payload. Usually contains signature/AVB information.
    // +---------+

    byte_view payload;
    byte_view tail;

    // MTK headers
    const mtk_hdr *k_hdr = nullptr;
    const mtk_hdr *r_hdr = nullptr;

    // The pointers/values after parse_image
    // +---------------+
    // | z_hdr         | z_info.hdr_sz
    // +---------------+
    // | kernel        | hdr->kernel_size()
    // +---------------+
    // | z_info.tail   | z_info.tail.sz()
    // +---------------+
    const zimage_hdr *z_hdr = nullptr;
    struct {
        uint32_t hdr_sz;
        byte_view tail;
    } z_info;

    // AVB structs
    const AvbFooter *avb_footer = nullptr;
    const AvbVBMetaImageHeader *vbmeta = nullptr;

    // Pointers to blocks defined in header
    const uint8_t *kernel = nullptr;
    const uint8_t *ramdisk = nullptr;
    const uint8_t *second = nullptr;
    const uint8_t *extra = nullptr;
    const uint8_t *recovery_dtbo = nullptr;
    const uint8_t *dtb = nullptr;
    const uint8_t *signature = nullptr;
    const uint8_t *vendor_ramdisk_table = nullptr;
    const uint8_t *bootconfig = nullptr;

    // dtb embedded in kernel
    byte_view kernel_dtb;

    // Blocks defined in header but we do not care
    byte_view ignore;

    boot_img(const char *);
    ~boot_img();

    bool parse_image(const uint8_t *addr, FileFormat type);
    std::pair<const uint8_t *, dyn_img_hdr *> create_hdr(const uint8_t *addr, FileFormat type);

    // Rust FFI
    rust::Slice<const uint8_t> get_payload() const { return payload; }
    rust::Slice<const uint8_t> get_tail() const { return tail; }
    bool verify(const char *cert = nullptr) const;
};
