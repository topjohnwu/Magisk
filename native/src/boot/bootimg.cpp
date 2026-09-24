module;
#include <memory>
#include <bitset>
#include <rust/cxx.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

export module boot;
import std;
export import base;

using namespace std;

#define PADDING 15
#define SHA256_DIGEST_SIZE 32
#define SHA_DIGEST_SIZE 20

#define RETURN_OK       0
#define RETURN_ERROR    1
#define RETURN_CHROMEOS 2

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

#define BUFFER_MATCH(buf, s) (memcmp(buf, s, sizeof(s) - 1) == 0)
#define BUFFER_CONTAIN(buf, sz, s) (memmem(buf, sz, s, sizeof(s) - 1) != nullptr)

#define BOOT_MAGIC      "ANDROID!"
#define VENDOR_BOOT_MAGIC "VNDRBOOT"
#define DTB_MAGIC       "\xd0\x0d\xfe\xed"
#define LG_BUMP_MAGIC   "\x41\xa9\xe4\x67\x74\x4d\x1d\x1b\xa4\x29\xf2\xec\xea\x65\x52\x79"
#define SEANDROID_MAGIC "SEANDROIDENFORCE"
#define NOOKHD_RL_MAGIC "Red Loader"
#define NOOKHD_GL_MAGIC "Green Loader"
#define NOOKHD_GR_MAGIC "Green Recovery"
#define NOOKHD_EB_MAGIC "eMMC boot.img+secondloader"
#define NOOKHD_ER_MAGIC "eMMC recovery.img+secondloader"
#define NOOKHD_PRE_HEADER_SZ 1048576
#define ACCLAIM_MAGIC   "BauwksBoot"
#define ACCLAIM_PRE_HEADER_SZ 262144
#define AMONET_MICROLOADER_MAGIC "microloader"
#define AMONET_MICROLOADER_SZ 1024
#define AVB_FOOTER_MAGIC "AVBf"
#define AVB_MAGIC "AVB0"

export {
#include "boot-rs.hpp"

enum class FileFormat : uint8_t;

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

template <typename T>
T align_to(T v, int a) {
    static_assert(std::is_integral_v<T>);
    return (v + a - 1) / a * a;
}

template <typename T>
T align_padding(T v, int a) {
    return align_to(v, a) - v;
}

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
    void print() const {
        uint32_t ver = header_version();
        fprintf(stderr, "%-*s [%u]\n", PADDING, "HEADER_VER", ver);
        if (!is_vendor())
            fprintf(stderr, "%-*s [%u]\n", PADDING, "KERNEL_SZ", kernel_size());
        fprintf(stderr, "%-*s [%u]\n", PADDING, "RAMDISK_SZ", ramdisk_size());
        if (ver < 3)
            fprintf(stderr, "%-*s [%u]\n", PADDING, "SECOND_SZ", second_size());
        if (ver == 0)
            fprintf(stderr, "%-*s [%u]\n", PADDING, "EXTRA_SZ", extra_size());
        if (ver == 1 || ver == 2)
            fprintf(stderr, "%-*s [%u]\n", PADDING, "RECOV_DTBO_SZ", recovery_dtbo_size());
        if (ver == 2 || is_vendor())
            fprintf(stderr, "%-*s [%u]\n", PADDING, "DTB_SZ", dtb_size());
        if (ver == 4 && is_vendor())
            fprintf(stderr, "%-*s [%u]\n", PADDING, "BOOTCONFIG_SZ", bootconfig_size());

        if (uint32_t os_ver = os_version()) {
            int a,b,c,y,m = 0;
            int version = os_ver >> 11;
            int patch_level = os_ver & 0x7ff;

            a = (version >> 14) & 0x7f;
            b = (version >> 7) & 0x7f;
            c = version & 0x7f;
            fprintf(stderr, "%-*s [%d.%d.%d]\n", PADDING, "OS_VERSION", a, b, c);

            y = (patch_level >> 4) + 2000;
            m = patch_level & 0xf;
            fprintf(stderr, "%-*s [%d-%02d]\n", PADDING, "OS_PATCH_LEVEL", y, m);
        }

        fprintf(stderr, "%-*s [%u]\n", PADDING, "PAGESIZE", page_size());
        if (const char *n = name()) {
            fprintf(stderr, "%-*s [%s]\n", PADDING, "NAME", n);
        }
        fprintf(stderr, "%-*s [%.*s%.*s]\n", PADDING, "CMDLINE",
                BOOT_ARGS_SIZE, cmdline(), BOOT_EXTRA_ARGS_SIZE, extra_cmdline());
        if (const char *checksum = id()) {
            fprintf(stderr, "%-*s [", PADDING, "CHECKSUM");
            for (int i = 0; i < SHA256_DIGEST_SIZE; ++i)
                fprintf(stderr, "%02hhx", checksum[i]);
            fprintf(stderr, "]\n");
        }
    }
    void dump_hdr_file() const {
        FILE *fp = xfopen(HEADER_FILE, "w");
        if (name())
            fprintf(fp, "name=%s\n", name());
        fprintf(fp, "cmdline=%.*s%.*s\n", BOOT_ARGS_SIZE, cmdline(), BOOT_EXTRA_ARGS_SIZE, extra_cmdline());
        uint32_t ver = os_version();
        if (ver) {
            int a, b, c, y, m;
            int version, patch_level;
            version = ver >> 11;
            patch_level = ver & 0x7ff;

            a = (version >> 14) & 0x7f;
            b = (version >> 7) & 0x7f;
            c = version & 0x7f;
            fprintf(fp, "os_version=%d.%d.%d\n", a, b, c);

            y = (patch_level >> 4) + 2000;
            m = patch_level & 0xf;
            fprintf(fp, "os_patch_level=%d-%02d\n", y, m);
        }
        fclose(fp);
    }
    void load_hdr_file() {
        parse_prop_file(HEADER_FILE, [=, this](Utf8CStr key, Utf8CStr value) -> bool {
            if (key == "name" && name()) {
                memset(name(), 0, 16);
                memcpy(name(), value.data(), value.length() > 15 ? 15 : value.length());
            } else if (key == "cmdline") {
                memset(cmdline(), 0, BOOT_ARGS_SIZE);
                memset(extra_cmdline(), 0, BOOT_EXTRA_ARGS_SIZE);
                if (value.length() > BOOT_ARGS_SIZE) {
                    memcpy(cmdline(), value.data(), BOOT_ARGS_SIZE);
                    auto len = std::min(value.length() - BOOT_ARGS_SIZE, (size_t) BOOT_EXTRA_ARGS_SIZE);
                    memcpy(extra_cmdline(), value.data() + BOOT_ARGS_SIZE, len);
                } else {
                    memcpy(cmdline(), value.data(), value.length());
                }
            } else if (key == "os_version") {
                int patch_level = os_version() & 0x7ff;
                int a, b, c;
                sscanf(value.data(), "%d.%d.%d", &a, &b, &c);
                os_version() = (((a << 14) | (b << 7) | c) << 11) | patch_level;
            } else if (key == "os_patch_level") {
                int os_ver = os_version() >> 11;
                int y, m;
                sscanf(value.data(), "%d-%d", &y, &m);
                y -= 2000;
                os_version() = (os_ver << 11) | (y << 4) | m;
            }
            return true;
        });
    }

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
explicit                                \
name(const void *p, ssize_t sz = -1) {  \
    if (sz < 0) sz = sizeof(hdr);       \
    raw = calloc(sizeof(hdr), 1);       \
    memcpy(raw, p, sz);                 \
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

struct ZImage;

}

using namespace std;

#define RETURN_VENDOR   3

FileFormat check_fmt(const void *buf, size_t len) {
    return check_fmt({static_cast<const uint8_t *>(buf), len});
}

int find_dtb_offset(const void *buf, size_t len) {
    return find_dtb_offset({static_cast<const uint8_t *>(buf), len});
}

bool decompress(FileFormat type, int fd, const void *in, size_t size) {
    return decompress_bytes(type, byte_view { in, size }, fd);
}

off_t compress_len(FileFormat type, byte_view in, int fd) {
    auto prev = lseek(fd, 0, SEEK_CUR);
    compress_bytes(type, in, fd);
    auto now = lseek(fd, 0, SEEK_CUR);
    return now - prev;
}

off_t compress_len_kernel(FileFormat type, byte_view in, int fd) {
    auto prev = lseek(fd, 0, SEEK_CUR);
    compress_bytes_kernel(type, in, fd);
    auto now = lseek(fd, 0, SEEK_CUR);
    return now - prev;
}

void dump(const void *buf, size_t size, const char *filename) {
    if (size == 0)
        return;
    int fd = creat(filename, 0644);
    xwrite(fd, buf, size);
    close(fd);
}

size_t restore(int fd, const char *filename) {
    int ifd = xopen(filename, O_RDONLY);
    size_t size = lseek(ifd, 0, SEEK_END);
    lseek(ifd, 0, SEEK_SET);
    xsendfile(fd, ifd, nullptr, size);
    close(ifd);
    return size;
}

bool check_env(const char *name) {
    const char *val = getenv(name);
    return val != nullptr && val == "true"sv;
}

FileFormat check_fmt_lg(const uint8_t *buf, unsigned sz) {
    FileFormat fmt = check_fmt(buf, sz);
    if (fmt == FileFormat::LZ4_LEGACY) {
        // We need to check if it is LZ4_LG
        uint32_t off = 4;
        uint32_t block_sz;
        while (off + sizeof(block_sz) <= sz) {
            memcpy(&block_sz, buf + off, sizeof(block_sz));
            off += sizeof(block_sz);
            if (off + block_sz > sz)
                return FileFormat::LZ4_LG;
            off += block_sz;
        }
    }
    return fmt;
}

#define CMD_MATCH(s) BUFFER_MATCH(h->cmdline, s)

const char *vendor_ramdisk_type(int type) {
    switch (type) {
    case VENDOR_RAMDISK_TYPE_PLATFORM:
        return "platform";
    case VENDOR_RAMDISK_TYPE_RECOVERY:
        return "recovery";
    case VENDOR_RAMDISK_TYPE_DLKM:
        return "dlkm";
    case VENDOR_RAMDISK_TYPE_NONE:
    default:
        return "none";
    }
}

#define assert_off() \
if ((addr + off) > (map.data() + map_end)) {      \
    fprintf(stderr, "Corrupted boot image!\n");   \
    return false;                                 \
}

#define get_block(name)                 \
name = addr + off;                      \
off += hdr->name##_size();              \
off = align_to(off, hdr->page_size());  \
assert_off()

export struct boot_img {
    // Memory map of the whole image
    const mmap_data map;

    // Android image header
    dyn_img_hdr *hdr = nullptr;

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

    std::unique_ptr<ZImage> z_info;

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

    explicit boot_img(const char *image) :
    map(image), k_fmt(FileFormat::UNKNOWN), r_fmt(FileFormat::UNKNOWN), e_fmt(FileFormat::UNKNOWN) {
        fprintf(stderr, "Parsing boot image: [%s]\n", image);
        for (const uint8_t *addr = map.data(); addr < map.data() + map.size(); ++addr) {
            FileFormat fmt = check_fmt(addr, map.size());
            switch (fmt) {
            case FileFormat::CHROMEOS:
                // chromeos require external signing
                flags[CHROMEOS_FLAG] = true;
                addr += 65535;
                break;
            case FileFormat::DHTB:
                flags[DHTB_FLAG] = true;
                flags[SEANDROID_FLAG] = true;
                fprintf(stderr, "DHTB_HDR\n");
                addr += sizeof(dhtb_hdr) - 1;
                break;
            case FileFormat::BLOB:
                flags[BLOB_FLAG] = true;
                fprintf(stderr, "TEGRA_BLOB\n");
                addr += sizeof(blob_hdr) - 1;
                break;
            case FileFormat::AOSP:
            case FileFormat::AOSP_VENDOR:
                if (parse_image(addr, fmt))
                    return;
                // fallthrough
            default:
                break;
            }
        }
        exit(RETURN_ERROR);
    }
    ~boot_img() {
        delete hdr;
    }

    bool parse_image(const uint8_t *addr, FileFormat type) {
        addr = parse_hdr(addr, type);
        if (hdr == nullptr) {
            fprintf(stderr, "Invalid boot image header!\n");
            return false;
        }

        if (const char *id = hdr->id()) {
            for (int i = SHA_DIGEST_SIZE + 4; i < SHA256_DIGEST_SIZE; ++i) {
                if (id[i]) {
                    flags[SHA256_FLAG] = true;
                    break;
                }
            }
        }

        hdr->print();

        size_t map_end = align_to(map.size(), getpagesize());
        size_t off = hdr->hdr_space();
        get_block(kernel);
        get_block(ramdisk);
        get_block(second);
        get_block(extra);
        get_block(recovery_dtbo);
        get_block(dtb);
        get_block(signature);
        get_block(vendor_ramdisk_table);
        get_block(bootconfig);

        payload = byte_view(addr, off);
        auto tail_addr = addr + off;
        tail = byte_view(tail_addr, map.data() + map_end - tail_addr);

        if (auto size = hdr->kernel_size()) {
            if (int dtb_off = find_dtb_offset(kernel, size); dtb_off > 0) {
                kernel_dtb = byte_view(kernel + dtb_off, size - dtb_off);
                hdr->kernel_size() = dtb_off;
                fprintf(stderr, "%-*s [%zu]\n", PADDING, "KERNEL_DTB_SZ", kernel_dtb.size());
            }

            k_fmt = check_fmt_lg(kernel, hdr->kernel_size());
            if (k_fmt == FileFormat::MTK) {
                fprintf(stderr, "MTK_KERNEL_HDR\n");
                flags[MTK_KERNEL] = true;
                k_hdr = reinterpret_cast<const mtk_hdr *>(kernel);
                fprintf(stderr, "%-*s [%u]\n", PADDING, "SIZE", k_hdr->size);
                fprintf(stderr, "%-*s [%s]\n", PADDING, "NAME", k_hdr->name);
                kernel += sizeof(mtk_hdr);
                hdr->kernel_size() -= sizeof(mtk_hdr);
                k_fmt = check_fmt_lg(kernel, hdr->kernel_size());
            }
            if (k_fmt == FileFormat::ZIMAGE) {
                z_info = ZImage::parse(byte_view(kernel, hdr->kernel_size()));
                if (z_info != nullptr) {
                    fprintf(stderr, "ZIMAGE_KERNEL\n");
                    flags[ZIMAGE_KERNEL] = true;
                    kernel = z_info->piggy.data();
                    if (z_info->fmt != FileFormat::GZIP) {
                        hdr->kernel_size() = z_info->piggy.size() - sizeof(uint32_t);
                    } else {
                        hdr->kernel_size() = z_info->piggy.size();
                    }
                    k_fmt = z_info->fmt;
                }
            }
            fprintf(stderr, "%-*s [%s]\n", PADDING, "KERNEL_FMT", fmt2name(k_fmt));
        }
        if (auto size = hdr->ramdisk_size()) {
            if (hdr->vendor_ramdisk_table_size()) {
                for (auto &it : vendor_ramdisk_tbl()) {
                    FileFormat fmt = check_fmt_lg(ramdisk + it.ramdisk_offset, it.ramdisk_size);
                    fprintf(stderr,
                            "%-*s name=[%s] type=[%s] size=[%u] fmt=[%s]\n", PADDING, "VND_RAMDISK",
                            it.ramdisk_name, vendor_ramdisk_type(it.ramdisk_type),
                            it.ramdisk_size, fmt2name(fmt));
                }
            } else {
                r_fmt = check_fmt_lg(ramdisk, size);
                if (r_fmt == FileFormat::MTK) {
                    fprintf(stderr, "MTK_RAMDISK_HDR\n");
                    flags[MTK_RAMDISK] = true;
                    r_hdr = reinterpret_cast<const mtk_hdr *>(ramdisk);
                    fprintf(stderr, "%-*s [%u]\n", PADDING, "SIZE", r_hdr->size);
                    fprintf(stderr, "%-*s [%s]\n", PADDING, "NAME", r_hdr->name);
                    ramdisk += sizeof(mtk_hdr);
                    hdr->ramdisk_size() -= sizeof(mtk_hdr);
                    r_fmt = check_fmt_lg(ramdisk, hdr->ramdisk_size());
                }
                fprintf(stderr, "%-*s [%s]\n", PADDING, "RAMDISK_FMT", fmt2name(r_fmt));
            }
        }
        if (auto size = hdr->extra_size()) {
            e_fmt = check_fmt_lg(extra, size);
            fprintf(stderr, "%-*s [%s]\n", PADDING, "EXTRA_FMT", fmt2name(e_fmt));
        }

        if (tail.size()) {
            // Check special flags
            if (tail.size() >= 16 && BUFFER_MATCH(tail.data(), SEANDROID_MAGIC)) {
                fprintf(stderr, "SAMSUNG_SEANDROID\n");
                flags[SEANDROID_FLAG] = true;
            } else if (tail.size() >= 16 && BUFFER_MATCH(tail.data(), LG_BUMP_MAGIC)) {
                fprintf(stderr, "LG_BUMP_IMAGE\n");
                flags[LG_BUMP_FLAG] = true;
            } else if (verify()) {
                fprintf(stderr, "AVB1_SIGNED\n");
                flags[AVB1_SIGNED_FLAG] = true;
            }

            // Find AVB footer
            const void *footer = tail.data() + tail.size() - sizeof(AvbFooter);
            if (BUFFER_MATCH(footer, AVB_FOOTER_MAGIC)) {
                avb_footer = static_cast<const AvbFooter*>(footer);
                // Double check if meta header exists
                const void *meta = payload.data() + __builtin_bswap64(avb_footer->vbmeta_offset);
                if (BUFFER_MATCH(meta, AVB_MAGIC)) {
                    fprintf(stderr, "VBMETA\n");
                    flags[AVB_FLAG] = true;
                    vbmeta = static_cast<const AvbVBMetaImageHeader*>(meta);
                }
            }
        }

        return true;
    }
    const uint8_t *parse_hdr(const uint8_t *addr, FileFormat type) {
        if (type == FileFormat::AOSP_VENDOR) {
            fprintf(stderr, "VENDOR_BOOT_HDR\n");
            auto h = reinterpret_cast<const boot_img_hdr_vnd_v3*>(addr);
            switch (h->header_version) {
            case 4:
                hdr = new dyn_img_vnd_v4(addr);
                break;
            default:
                hdr = new dyn_img_vnd_v3(addr);
                break;
            }
            return addr;
        }

        auto h = reinterpret_cast<const boot_img_hdr_v0*>(addr);

        if (h->page_size >= 0x02000000) {
            fprintf(stderr, "PXA_BOOT_HDR\n");
            hdr = new dyn_img_pxa(addr);
            return addr;
        }

        auto make_aosp_hdr = [](const uint8_t *ptr, ssize_t size = -1) -> dyn_img_hdr * {
            auto h = reinterpret_cast<const boot_img_hdr_v0*>(ptr);
            if (memcmp(h->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE) != 0)
                return nullptr;

            switch (h->header_version) {
            case 1:
                return new dyn_img_v1(ptr, size);
            case 2:
                return new dyn_img_v2(ptr, size);
            case 3:
                return new dyn_img_v3(ptr, size);
            case 4:
                return new dyn_img_v4(ptr, size);
            default:
                return new dyn_img_v0(ptr, size);
            }
        };

        // For NOOKHD and ACCLAIM, the entire boot image is shifted by a fixed offset.
        // For AMONET, the header itself is internally shifted by a fixed offset.

        if (BUFFER_CONTAIN(addr, AMONET_MICROLOADER_SZ, AMONET_MICROLOADER_MAGIC) &&
            BUFFER_MATCH(addr + AMONET_MICROLOADER_SZ, BOOT_MAGIC)) {
            flags[AMONET_FLAG] = true;
            fprintf(stderr, "AMONET_MICROLOADER\n");

            // The real header is shifted
            h = reinterpret_cast<const boot_img_hdr_v0*>(addr + AMONET_MICROLOADER_SZ);
            auto real_hdr_sz = h->page_size - AMONET_MICROLOADER_SZ;
            hdr = make_aosp_hdr(addr + AMONET_MICROLOADER_SZ, real_hdr_sz);
            return addr;
        }

        if (CMD_MATCH(NOOKHD_RL_MAGIC) ||
            CMD_MATCH(NOOKHD_GL_MAGIC) ||
            CMD_MATCH(NOOKHD_GR_MAGIC) ||
            CMD_MATCH(NOOKHD_EB_MAGIC) ||
            CMD_MATCH(NOOKHD_ER_MAGIC)) {
            flags[NOOKHD_FLAG] = true;
            fprintf(stderr, "NOOKHD_LOADER\n");
            addr += NOOKHD_PRE_HEADER_SZ;
        } else if (BUFFER_MATCH(h->name, ACCLAIM_MAGIC)) {
            flags[ACCLAIM_FLAG] = true;
            fprintf(stderr, "ACCLAIM_LOADER\n");
            addr += ACCLAIM_PRE_HEADER_SZ;
        }

        hdr = make_aosp_hdr(addr);
        return addr;
    }
    std::span<const vendor_ramdisk_table_entry_v4> vendor_ramdisk_tbl() const {
        if (hdr->vendor_ramdisk_table_size() == 0) {
            return {};
        }

        // v4 vendor boot contains multiple ramdisks
        using table_entry = const vendor_ramdisk_table_entry_v4;
        if (hdr->vendor_ramdisk_table_entry_size() != sizeof(table_entry)) {
            fprintf(stderr,
                    "! Invalid vendor image: vendor_ramdisk_table_entry_size != %zu\n",
                    sizeof(table_entry));
            exit(RETURN_ERROR);
        }
        return span(reinterpret_cast<table_entry *>(vendor_ramdisk_table), hdr->vendor_ramdisk_table_entry_num());
    }

    // Rust FFI
    static std::unique_ptr<boot_img> create(Utf8CStr name) { return std::make_unique<boot_img>(name.c_str()); }
    rust::Slice<const uint8_t> get_payload() const { return payload; }
    rust::Slice<const uint8_t> get_tail() const { return tail; }
    bool is_signed() const { return flags[AVB1_SIGNED_FLAG]; }
    uint64_t tail_off() const { return tail.data() - map.data(); }

    // Implemented in Rust
    bool verify() const noexcept;
};

export int split_image_dtb(Utf8CStr filename, bool skip_decomp = false) {
    mmap_data img(filename.c_str());

    if (int offset = find_dtb_offset(img.data(), img.size()); offset > 0) {
        size_t off = (size_t) offset;

        FileFormat fmt = check_fmt_lg(img.data(), img.size());
        if (!skip_decomp && fmt_compressed(fmt)) {
            int fd = creat(KERNEL_FILE, 0644);
            if (!decompress(fmt, fd, img.data(), off)) {
                close(fd);
                unlink(KERNEL_FILE);
                return 1;
            }
            close(fd);
        } else {
            dump(img.data(), off, KERNEL_FILE);
        }
        dump(img.data() + off, img.size() - off, KER_DTB_FILE);
        return 0;
    } else {
        fprintf(stderr, "Cannot find DTB in %s\n", filename.c_str());
        return 1;
    }
}

export int unpack(Utf8CStr image, bool skip_decomp = false, bool hdr = false) {
    const boot_img boot(image.c_str());

    if (hdr)
        boot.hdr->dump_hdr_file();

    // Dump kernel
    if (!skip_decomp && fmt_compressed(boot.k_fmt)) {
        if (boot.hdr->kernel_size() != 0) {
            int fd = creat(KERNEL_FILE, 0644);
            if (!decompress(boot.k_fmt, fd, boot.kernel, boot.hdr->kernel_size())) {
                close(fd);
                unlink(KERNEL_FILE);
                return RETURN_ERROR;
            }
            close(fd);
        }
    } else {
        dump(boot.kernel, boot.hdr->kernel_size(), KERNEL_FILE);
    }

    // Dump kernel_dtb
    dump(boot.kernel_dtb.data(), boot.kernel_dtb.size(), KER_DTB_FILE);

    // Dump ramdisk
    if (boot.hdr->vendor_ramdisk_table_size()) {
        xmkdir(VND_RAMDISK_DIR, 0755);
        owned_fd dirfd = xopen(VND_RAMDISK_DIR, O_RDONLY | O_CLOEXEC);
        for (auto &it : boot.vendor_ramdisk_tbl()) {
            char file_name[40];
            if (it.ramdisk_name[0] == '\0') {
                strscpy(file_name, RAMDISK_FILE, sizeof(file_name));
            } else {
                ssprintf(file_name, sizeof(file_name), "%s.cpio", it.ramdisk_name);
            }
            owned_fd fd = xopenat(dirfd, file_name, O_CREAT | O_TRUNC | O_WRONLY | O_CLOEXEC, 0644);
            FileFormat fmt = check_fmt_lg(boot.ramdisk + it.ramdisk_offset, it.ramdisk_size);
            if (!skip_decomp && fmt_compressed(fmt)) {
                if (!decompress(fmt, fd, boot.ramdisk + it.ramdisk_offset, it.ramdisk_size)) {
                    unlinkat(dirfd, file_name, 0);
                    return RETURN_ERROR;
                }
            } else {
                xwrite(fd, boot.ramdisk + it.ramdisk_offset, it.ramdisk_size);
            }
        }
    } else if (!skip_decomp && fmt_compressed(boot.r_fmt)) {
        if (boot.hdr->ramdisk_size() != 0) {
            int fd = creat(RAMDISK_FILE, 0644);
            if (!decompress(boot.r_fmt, fd, boot.ramdisk, boot.hdr->ramdisk_size())) {
                close(fd);
                unlink(RAMDISK_FILE);
                return RETURN_ERROR;
            }
            close(fd);
        }
    } else {
        dump(boot.ramdisk, boot.hdr->ramdisk_size(), RAMDISK_FILE);
    }

    // Dump second
    dump(boot.second, boot.hdr->second_size(), SECOND_FILE);

    // Dump extra
    if (!skip_decomp && fmt_compressed(boot.e_fmt)) {
        if (boot.hdr->extra_size() != 0) {
            int fd = creat(EXTRA_FILE, 0644);
            if (!decompress(boot.e_fmt, fd, boot.extra, boot.hdr->extra_size())) {
                close(fd);
                unlink(EXTRA_FILE);
                return RETURN_ERROR;
            }
            close(fd);
        }
    } else {
        dump(boot.extra, boot.hdr->extra_size(), EXTRA_FILE);
    }

    // Dump recovery_dtbo
    dump(boot.recovery_dtbo, boot.hdr->recovery_dtbo_size(), RECV_DTBO_FILE);

    // Dump dtb
    dump(boot.dtb, boot.hdr->dtb_size(), DTB_FILE);

    // Dump bootconfig
    dump(boot.bootconfig, boot.hdr->bootconfig_size(), BOOTCONFIG_FILE);

    if (boot.flags[CHROMEOS_FLAG]) return RETURN_CHROMEOS;
    if (boot.hdr->is_vendor()) return RETURN_VENDOR;
    return RETURN_OK;
}

#define file_align_with(page_size) \
write_zero(fd, align_padding(lseek(fd, 0, SEEK_CUR) - off.header, page_size))

#define file_align() file_align_with(boot.hdr->page_size())

export void repack(Utf8CStr src_img, Utf8CStr out_img, bool skip_comp = false) {
    const boot_img boot(src_img.c_str());
    fprintf(stderr, "Repack to boot image: [%s]\n", out_img.c_str());

    struct {
        uint32_t header;
        uint32_t kernel;
        uint32_t ramdisk;
        uint32_t second;
        uint32_t extra;
        uint32_t dtb;
        uint32_t tail;
        uint32_t vbmeta;
    } off{};

    // Create a new boot header and reset sizes
    auto hdr = boot.hdr->clone();
    hdr->kernel_size() = 0;
    hdr->ramdisk_size() = 0;
    hdr->second_size() = 0;
    hdr->dtb_size() = 0;
    hdr->bootconfig_size() = 0;

    if (access(HEADER_FILE, R_OK) == 0)
        hdr->load_hdr_file();

    /***************
     * Write blocks
     ***************/

    // Create new image
    int fd = open(out_img.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);

    // Copy non-standard headers
    if (boot.flags[DHTB_FLAG]) {
        xwrite(fd, boot.map.data(), sizeof(dhtb_hdr));
    } else if (boot.flags[BLOB_FLAG]) {
        xwrite(fd, boot.map.data(), sizeof(blob_hdr));
    } else if (boot.flags[NOOKHD_FLAG]) {
        xwrite(fd, boot.map.data(), NOOKHD_PRE_HEADER_SZ);
    } else if (boot.flags[ACCLAIM_FLAG]) {
        xwrite(fd, boot.map.data(), ACCLAIM_PRE_HEADER_SZ);
    }

    // Copy raw header
    off.header = lseek(fd, 0, SEEK_CUR);
    xwrite(fd, boot.payload.data(), hdr->hdr_space());

    // kernel
    off.kernel = lseek(fd, 0, SEEK_CUR);
    if (boot.flags[MTK_KERNEL]) {
        // Copy MTK headers
        xwrite(fd, boot.k_hdr, sizeof(mtk_hdr));
    }
    if (boot.flags[ZIMAGE_KERNEL]) {
        // Copy zImage headers stub
        xwrite(fd, boot.z_info->head.data(), boot.z_info->head.size());
    }
    uint32_t z_payload_sz = 0;
    if (access(KERNEL_FILE, R_OK) == 0) {
        mmap_data m(KERNEL_FILE);
        uint32_t payload_sz = 0;
        if (!skip_comp && !fmt_compressed_any(check_fmt(m.data(), m.size())) && fmt_compressed(boot.k_fmt)) {
            payload_sz = compress_len_kernel(boot.k_fmt, m, fd);
            if (boot.flags[ZIMAGE_KERNEL] && boot.k_fmt != FileFormat::GZIP) {
                // For non-gzip compression in zImage, size_append appends the 4-byte LE uncompressed size
                uint32_t sz = m.size();
                xwrite(fd, &sz, sizeof(sz));
                payload_sz += sizeof(sz);
            }
            hdr->kernel_size() = payload_sz;
        } else {
            payload_sz = xwrite(fd, m.data(), m.size());
            hdr->kernel_size() = payload_sz;
        }

        if (boot.flags[ZIMAGE_KERNEL]) {
            z_payload_sz = payload_sz;
            auto tail_buf = boot.z_info->new_tail(payload_sz);
            xwrite(fd, tail_buf.data(), tail_buf.size());
            hdr->kernel_size() = boot.z_info->head.size() + payload_sz + tail_buf.size();
        }
    } else if (boot.hdr->kernel_size() != 0) {
        if (boot.flags[ZIMAGE_KERNEL]) {
            xwrite(fd, boot.z_info->piggy.data(), boot.z_info->piggy.size());
            xwrite(fd, boot.z_info->tail.data(), boot.z_info->tail.size());
            hdr->kernel_size() = boot.z_info->head.size() + boot.z_info->piggy.size() + boot.z_info->tail.size();
        } else {
            xwrite(fd, boot.kernel, boot.hdr->kernel_size());
            hdr->kernel_size() = boot.hdr->kernel_size();
        }
    }

    // kernel dtb
    if (access(KER_DTB_FILE, R_OK) == 0)
        hdr->kernel_size() += restore(fd, KER_DTB_FILE);
    file_align();

    // ramdisk
    off.ramdisk = lseek(fd, 0, SEEK_CUR);
    if (boot.flags[MTK_RAMDISK]) {
        // Copy MTK headers
        xwrite(fd, boot.r_hdr, sizeof(mtk_hdr));
    }

    vector<vendor_ramdisk_table_entry_v4> ramdisk_table;

    if (boot.hdr->vendor_ramdisk_table_size()) {
        // Create a copy so we can modify it
        ramdisk_table.assign_range(boot.vendor_ramdisk_tbl());

        owned_fd dirfd = xopen(VND_RAMDISK_DIR, O_RDONLY | O_CLOEXEC);
        uint32_t ramdisk_offset = 0;
        for (auto &it : ramdisk_table) {
            char file_name[64];
            if (it.ramdisk_name[0] == '\0') {
                strscpy(file_name, RAMDISK_FILE, sizeof(file_name));
            } else {
                ssprintf(file_name, sizeof(file_name), "%s.cpio", it.ramdisk_name);
            }
            mmap_data m(dirfd, file_name);
            FileFormat fmt = check_fmt_lg(boot.ramdisk + it.ramdisk_offset, it.ramdisk_size);
            it.ramdisk_offset = ramdisk_offset;
            if (!skip_comp && !fmt_compressed_any(check_fmt(m.data(), m.size())) && fmt_compressed(fmt)) {
                it.ramdisk_size = compress_len(fmt, m, fd);
            } else {
                it.ramdisk_size = xwrite(fd, m.data(), m.size());
            }
            ramdisk_offset += it.ramdisk_size;
        }

        hdr->ramdisk_size() = ramdisk_offset;
        file_align();
    } else if (access(RAMDISK_FILE, R_OK) == 0) {
        mmap_data m(RAMDISK_FILE);
        auto r_fmt = boot.r_fmt;
        if (!skip_comp && !hdr->is_vendor() && hdr->header_version() == 4 && r_fmt != FileFormat::LZ4_LEGACY) {
            // A v4 boot image ramdisk will have to be merged with other vendor ramdisks,
            // and they have to use the exact same compression method. v4 GKIs are required to
            // use lz4 (legacy), so hardcode the format here.
            fprintf(stderr, "RAMDISK_FMT: [%s] -> [%s]\n", fmt2name(r_fmt), fmt2name(FileFormat::LZ4_LEGACY));
            r_fmt = FileFormat::LZ4_LEGACY;
        }
        if (!skip_comp && !fmt_compressed_any(check_fmt(m.data(), m.size())) && fmt_compressed(r_fmt)) {
            hdr->ramdisk_size() = compress_len(r_fmt, m, fd);
        } else {
            hdr->ramdisk_size() = xwrite(fd, m.data(), m.size());
        }
        file_align();
    }

    // second
    off.second = lseek(fd, 0, SEEK_CUR);
    if (access(SECOND_FILE, R_OK) == 0) {
        hdr->second_size() = restore(fd, SECOND_FILE);
        file_align();
    }

    // extra
    off.extra = lseek(fd, 0, SEEK_CUR);
    if (access(EXTRA_FILE, R_OK) == 0) {
        mmap_data m(EXTRA_FILE);
        if (!skip_comp && !fmt_compressed_any(check_fmt(m.data(), m.size())) && fmt_compressed(boot.e_fmt)) {
            hdr->extra_size() = compress_len(boot.e_fmt, m, fd);
        } else {
            hdr->extra_size() = xwrite(fd, m.data(), m.size());
        }
        file_align();
    }

    // recovery_dtbo
    if (access(RECV_DTBO_FILE, R_OK) == 0) {
        hdr->recovery_dtbo_offset() = lseek(fd, 0, SEEK_CUR);
        hdr->recovery_dtbo_size() = restore(fd, RECV_DTBO_FILE);
        file_align();
    }

    // dtb
    off.dtb = lseek(fd, 0, SEEK_CUR);
    if (access(DTB_FILE, R_OK) == 0) {
        hdr->dtb_size() = restore(fd, DTB_FILE);
        file_align();
    }

    // Copy boot signature
    if (boot.hdr->signature_size()) {
        xwrite(fd, boot.signature, boot.hdr->signature_size());
        file_align();
    }

    // vendor ramdisk table
    if (!ramdisk_table.empty()) {
        xwrite(fd, ramdisk_table.data(), sizeof(*ramdisk_table.data()) * ramdisk_table.size());
        file_align();
    }

    // bootconfig
    if (access(BOOTCONFIG_FILE, R_OK) == 0) {
        hdr->bootconfig_size() = restore(fd, BOOTCONFIG_FILE);
        file_align();
    }

    // Proprietary stuffs
    if (boot.flags[SEANDROID_FLAG]) {
        xwrite(fd, SEANDROID_MAGIC, 16);
        if (boot.flags[DHTB_FLAG]) {
            xwrite(fd, "\xFF\xFF\xFF\xFF", 4);
        }
    } else if (boot.flags[LG_BUMP_FLAG]) {
        xwrite(fd, LG_BUMP_MAGIC, 16);
    }

    off.tail = lseek(fd, 0, SEEK_CUR);
    file_align();

    // vbmeta
    if (boot.flags[AVB_FLAG]) {
        // According to avbtool.py, if the input is not an Android sparse image
        // (which boot images are not), the default block size is 4096
        file_align_with(4096);
        off.vbmeta = lseek(fd, 0, SEEK_CUR);
        uint64_t vbmeta_size = __builtin_bswap64(boot.avb_footer->vbmeta_size);
        xwrite(fd, boot.vbmeta, vbmeta_size);
    }

    // Pad image to original size if not chromeos (as it requires post processing)
    if (!boot.flags[CHROMEOS_FLAG]) {
        off_t current = lseek(fd, 0, SEEK_CUR);
        if (current < boot.map.size()) {
            write_zero(fd, boot.map.size() - current);
        }
    }

    /******************
     * Patch the image
     ******************/

    uint32_t aosp_img_size = off.tail - off.header;

    // Map output image as rw
    mmap_data out(fd, lseek(fd, 0, SEEK_END), true);

    // MTK headers
    if (boot.flags[MTK_KERNEL]) {
        auto m_hdr = reinterpret_cast<mtk_hdr *>(out.data() + off.kernel);
        m_hdr->size = hdr->kernel_size();
        hdr->kernel_size() += sizeof(mtk_hdr);
    }
    if (boot.flags[MTK_RAMDISK]) {
        auto m_hdr = reinterpret_cast<mtk_hdr *>(out.data() + off.ramdisk);
        m_hdr->size = hdr->ramdisk_size();
        hdr->ramdisk_size() += sizeof(mtk_hdr);
    }

    // zImage header stub
    if (boot.flags[ZIMAGE_KERNEL] && z_payload_sz) {
        auto head_stub = boot.z_info->new_head(z_payload_sz);
        uint8_t *head_ptr = out.data() + off.kernel + (boot.flags[MTK_KERNEL] ? sizeof(mtk_hdr) : 0);
        memcpy(head_ptr, head_stub.data(), head_stub.size());
    }

    // Make sure header size matches
    hdr->header_size() = hdr->hdr_size();

    // Update checksum
    if (char *id = hdr->id()) {
        auto ctx = get_sha(!boot.flags[SHA256_FLAG]);
        uint32_t size = hdr->kernel_size();
        ctx->update(byte_view(out.data() + off.kernel, size));
        ctx->update(byte_view(&size, sizeof(size)));
        size = hdr->ramdisk_size();
        ctx->update(byte_view(out.data() + off.ramdisk, size));
        ctx->update(byte_view(&size, sizeof(size)));
        size = hdr->second_size();
        ctx->update(byte_view(out.data() + off.second, size));
        ctx->update(byte_view(&size, sizeof(size)));
        size = hdr->extra_size();
        if (size) {
            ctx->update(byte_view(out.data() + off.extra, size));
            ctx->update(byte_view(&size, sizeof(size)));
        }
        uint32_t ver = hdr->header_version();
        if (ver == 1 || ver == 2) {
            size = hdr->recovery_dtbo_size();
            ctx->update(byte_view(out.data() + hdr->recovery_dtbo_offset(), size));
            ctx->update(byte_view(&size, sizeof(size)));
        }
        if (ver == 2) {
            size = hdr->dtb_size();
            ctx->update(byte_view(out.data() + off.dtb, size));
            ctx->update(byte_view(&size, sizeof(size)));
        }
        memset(id, 0, BOOT_ID_SIZE);
        ctx->finalize_into(byte_data(id, ctx->output_size()));
    }

    // Print new header info
    hdr->print();

    // Copy main header
    if (boot.flags[AMONET_FLAG]) {
        auto real_hdr_sz = std::min(hdr->hdr_space() - AMONET_MICROLOADER_SZ, hdr->hdr_size());
        memcpy(out.data() + off.header + AMONET_MICROLOADER_SZ, hdr->raw_hdr(), real_hdr_sz);
    } else {
        memcpy(out.data() + off.header, hdr->raw_hdr(), hdr->hdr_size());
    }

    if (boot.flags[AVB_FLAG]) {
        // Copy and patch AVB structures
        auto footer = reinterpret_cast<AvbFooter*>(out.data() + out.size() - sizeof(AvbFooter));
        memcpy(footer, boot.avb_footer, sizeof(AvbFooter));
        footer->original_image_size = __builtin_bswap64(aosp_img_size);
        footer->vbmeta_offset = __builtin_bswap64(off.vbmeta);
        if (check_env("PATCHVBMETAFLAG")) {
            auto vbmeta = reinterpret_cast<AvbVBMetaImageHeader*>(out.data() + off.vbmeta);
            vbmeta->flags = __builtin_bswap32(3);
        }
    }

    if (boot.flags[DHTB_FLAG]) {
        // DHTB header
        auto d_hdr = reinterpret_cast<dhtb_hdr *>(out.data());
        d_hdr->size = aosp_img_size + 16 /* SEANDROID_MAGIC */ + 4 /* DHTB trailer */;
        sha256_hash(byte_view(out.data() + sizeof(dhtb_hdr), d_hdr->size),
                    byte_data(d_hdr->checksum, SHA256_DIGEST_SIZE));
    } else if (boot.flags[BLOB_FLAG]) {
        // Blob header
        auto b_hdr = reinterpret_cast<blob_hdr *>(out.data());
        b_hdr->size = aosp_img_size;
    }

    // Sign the image after we finish patching the boot image
    if (boot.flags[AVB1_SIGNED_FLAG]) {
        byte_view payload(out.data() + off.header, aosp_img_size);
        auto sig = sign_payload(payload);
        if (!sig.empty()) {
            lseek(fd, off.tail, SEEK_SET);
            xwrite(fd, sig.data(), sig.size());
        }
    }

    close(fd);
}

export void cleanup() {
    unlink(HEADER_FILE);
    unlink(KERNEL_FILE);
    unlink(RAMDISK_FILE);
    unlink(SECOND_FILE);
    unlink(KER_DTB_FILE);
    unlink(EXTRA_FILE);
    unlink(RECV_DTBO_FILE);
    unlink(DTB_FILE);
    unlink(BOOTCONFIG_FILE);
    rm_rf(VND_RAMDISK_DIR);
}
