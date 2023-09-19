#include <bit>
#include <functional>
#include <memory>

#include <base.hpp>

#include "boot-rs.hpp"
#include "bootimg.hpp"
#include "magiskboot.hpp"
#include "compress.hpp"

using namespace std;

#define PADDING 15
#define SHA256_DIGEST_SIZE 32
#define SHA_DIGEST_SIZE 20

static void decompress(format_t type, int fd, const void *in, size_t size) {
    auto ptr = get_decoder(type, make_unique<fd_channel>(fd));
    ptr->write(in, size);
}

static off_t compress(format_t type, int fd, const void *in, size_t size) {
    auto prev = lseek(fd, 0, SEEK_CUR);
    {
        auto strm = get_encoder(type, make_unique<fd_channel>(fd));
        strm->write(in, size);
    }
    auto now = lseek(fd, 0, SEEK_CUR);
    return now - prev;
}

static void dump(const void *buf, size_t size, const char *filename) {
    if (size == 0)
        return;
    int fd = creat(filename, 0644);
    xwrite(fd, buf, size);
    close(fd);
}

static size_t restore(int fd, const char *filename) {
    int ifd = xopen(filename, O_RDONLY);
    size_t size = lseek(ifd, 0, SEEK_END);
    lseek(ifd, 0, SEEK_SET);
    xsendfile(fd, ifd, nullptr, size);
    close(ifd);
    return size;
}

void dyn_img_hdr::print() const {
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

void dyn_img_hdr::dump_hdr_file() const {
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

void dyn_img_hdr::load_hdr_file() {
    parse_prop_file(HEADER_FILE, [=, this](string_view key, string_view value) -> bool {
        if (key == "name" && name()) {
            memset(name(), 0, 16);
            memcpy(name(), value.data(), value.length() > 15 ? 15 : value.length());
        } else if (key == "cmdline") {
            memset(cmdline(), 0, BOOT_ARGS_SIZE);
            memset(extra_cmdline(), 0, BOOT_EXTRA_ARGS_SIZE);
            if (value.length() > BOOT_ARGS_SIZE) {
                memcpy(cmdline(), value.data(), BOOT_ARGS_SIZE);
                auto len = std::min(value.length() - BOOT_ARGS_SIZE, (size_t) BOOT_EXTRA_ARGS_SIZE);
                memcpy(extra_cmdline(), &value[BOOT_ARGS_SIZE], len);
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

boot_img::boot_img(const char *image) : map(image) {
    fprintf(stderr, "Parsing boot image: [%s]\n", image);
    for (const uint8_t *addr = map.buf(); addr < map.buf() + map.sz(); ++addr) {
        format_t fmt = check_fmt(addr, map.sz());
        switch (fmt) {
        case CHROMEOS:
            // chromeos require external signing
            flags[CHROMEOS_FLAG] = true;
            addr += 65535;
            break;
        case DHTB:
            flags[DHTB_FLAG] = true;
            flags[SEANDROID_FLAG] = true;
            fprintf(stderr, "DHTB_HDR\n");
            addr += sizeof(dhtb_hdr) - 1;
            break;
        case BLOB:
            flags[BLOB_FLAG] = true;
            fprintf(stderr, "TEGRA_BLOB\n");
            addr += sizeof(blob_hdr) - 1;
            break;
        case AOSP:
        case AOSP_VENDOR:
            if (parse_image(addr, fmt))
                return;
            // fallthrough
        default:
            break;
        }
    }
    exit(1);
}

boot_img::~boot_img() {
    delete hdr;
}

struct [[gnu::packed]] fdt_header {
    struct fdt32_t {
        uint32_t byte0: 8;
        uint32_t byte1: 8;
        uint32_t byte2: 8;
        uint32_t byte3: 8;

        constexpr operator uint32_t() const {
            return bit_cast<uint32_t>(fdt32_t {
                .byte0 = byte3,
                .byte1 = byte2,
                .byte2 = byte1,
                .byte3 = byte0
            });
        }
    };

    struct node_header {
        fdt32_t tag;
        char name[0];
    };

    fdt32_t magic;			 /* magic word FDT_MAGIC */
    fdt32_t totalsize;		 /* total size of DT block */
    fdt32_t off_dt_struct;		 /* offset to structure */
    fdt32_t off_dt_strings;		 /* offset to strings */
    fdt32_t off_mem_rsvmap;		 /* offset to memory reserve map */
    fdt32_t version;		 /* format version */
    fdt32_t last_comp_version;	 /* last compatible version */

    /* version 2 fields below */
    fdt32_t boot_cpuid_phys;	 /* Which physical CPU id we're
					    booting on */
    /* version 3 fields below */
    fdt32_t size_dt_strings;	 /* size of the strings block */

    /* version 17 fields below */
    fdt32_t size_dt_struct;		 /* size of the structure block */
};


static int find_dtb_offset(const uint8_t *buf, unsigned sz) {
    const uint8_t * const end = buf + sz;

    for (auto curr = buf; curr < end; curr += sizeof(fdt_header)) {
        curr = static_cast<uint8_t*>(memmem(curr, end - curr, DTB_MAGIC, sizeof(fdt_header::fdt32_t)));
        if (curr == nullptr)
            return -1;

        auto fdt_hdr = reinterpret_cast<const fdt_header *>(curr);

        // Check that fdt_header.totalsize does not overflow kernel image size
        uint32_t totalsize = fdt_hdr->totalsize;
        if (totalsize > end - curr)
            continue;

        // Check that fdt_header.off_dt_struct does not overflow kernel image size
        uint32_t off_dt_struct = fdt_hdr->off_dt_struct;
        if (off_dt_struct > end - curr)
            continue;

        // Check that fdt_node_header.tag of first node is FDT_BEGIN_NODE
        auto fdt_node_hdr = reinterpret_cast<const fdt_header::node_header *>(curr + off_dt_struct);
        if (fdt_node_hdr->tag != 0x1u)
            continue;

        return curr - buf;
    }
    return -1;
}

static format_t check_fmt_lg(const uint8_t *buf, unsigned sz) {
    format_t fmt = check_fmt(buf, sz);
    if (fmt == LZ4_LEGACY) {
        // We need to check if it is LZ4_LG
        uint32_t off = 4;
        uint32_t block_sz;
        while (off + sizeof(block_sz) <= sz) {
            memcpy(&block_sz, buf + off, sizeof(block_sz));
            off += sizeof(block_sz);
            if (off + block_sz > sz)
                return LZ4_LG;
            off += block_sz;
        }
    }
    return fmt;
}

#define CMD_MATCH(s) BUFFER_MATCH(h->cmdline, s)

const pair<const uint8_t *, dyn_img_hdr *>
        boot_img::create_hdr(const uint8_t *addr, format_t type) {
    if (type == AOSP_VENDOR) {
        fprintf(stderr, "VENDOR_BOOT_HDR\n");
        auto h = reinterpret_cast<const boot_img_hdr_vnd_v3*>(addr);
        switch (h->header_version) {
        case 4:
            return make_pair(addr, new dyn_img_vnd_v4(addr));
        default:
            return make_pair(addr, new dyn_img_vnd_v3(addr));
        }
    }

    auto h = reinterpret_cast<const boot_img_hdr_v0*>(addr);

    if (h->page_size >= 0x02000000) {
        fprintf(stderr, "PXA_BOOT_HDR\n");
        return make_pair(addr, new dyn_img_pxa(addr));
    }

    auto make_hdr = [](const uint8_t *ptr) -> dyn_img_hdr * {
        auto h = reinterpret_cast<const boot_img_hdr_v0*>(ptr);
        if (memcmp(h->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE) != 0)
            return nullptr;

        switch (h->header_version) {
        case 1:
            return new dyn_img_v1(ptr);
        case 2:
            return new dyn_img_v2(ptr);
        case 3:
            return new dyn_img_v3(ptr);
        case 4:
            return new dyn_img_v4(ptr);
        default:
            return new dyn_img_v0(ptr);
        }
    };

    // For NOOKHD and ACCLAIM, the entire boot image is shifted by a fixed offset.
    // For AMONET, only the header is internally shifted by a fixed offset.

    if (BUFFER_CONTAIN(addr, AMONET_MICROLOADER_SZ, AMONET_MICROLOADER_MAGIC) &&
        BUFFER_MATCH(addr + AMONET_MICROLOADER_SZ, BOOT_MAGIC)) {
        flags[AMONET_FLAG] = true;
        fprintf(stderr, "AMONET_MICROLOADER\n");

        // The real header is shifted, copy to temporary buffer
        h = reinterpret_cast<const boot_img_hdr_v0*>(addr + AMONET_MICROLOADER_SZ);
        if (memcmp(h->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE) != 0)
            return make_pair(addr, nullptr);

        auto real_hdr_sz = h->page_size - AMONET_MICROLOADER_SZ;
        heap_data copy(h->page_size);
        memcpy(copy.buf(), h, real_hdr_sz);

        return make_pair(addr, make_hdr(copy.buf()));
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

    // addr could be adjusted
    return make_pair(addr, make_hdr(addr));
}

#define assert_off() \
if ((base_addr + off) > (map.buf() + map.sz())) { \
    fprintf(stderr, "Corrupted boot image!\n");   \
    return false;    \
}

#define get_block(name)                 \
name = base_addr + off;                 \
off += hdr->name##_size();              \
off = align_to(off, hdr->page_size());  \
assert_off();

#define get_ignore(name)                                            \
if (hdr->name##_size()) {                                           \
    auto blk_sz = align_to(hdr->name##_size(), hdr->page_size());   \
    off += blk_sz;                                                  \
}                                                                   \
assert_off();


bool boot_img::parse_image(const uint8_t *p, format_t type) {
    auto [base_addr, hdr] = create_hdr(p, type);
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

    size_t off = hdr->hdr_space();
    get_block(kernel);
    get_block(ramdisk);
    get_block(second);
    get_block(extra);
    get_block(recovery_dtbo);
    get_block(dtb);

    auto ignore_addr = base_addr + off;
    get_ignore(signature)
    get_ignore(vendor_ramdisk_table)
    get_ignore(bootconfig)

    payload = byte_view(base_addr, off);
    auto tail_addr = base_addr + off;
    ignore = byte_view(ignore_addr, tail_addr - ignore_addr);
    tail = byte_view(tail_addr, map.buf() + map.sz() - tail_addr);

    if (auto size = hdr->kernel_size()) {
        if (int dtb_off = find_dtb_offset(kernel, size); dtb_off > 0) {
            kernel_dtb = byte_view(kernel + dtb_off, size - dtb_off);
            hdr->kernel_size() = dtb_off;
            fprintf(stderr, "%-*s [%zu]\n", PADDING, "KERNEL_DTB_SZ", kernel_dtb.sz());
        }

        k_fmt = check_fmt_lg(kernel, hdr->kernel_size());
        if (k_fmt == MTK) {
            fprintf(stderr, "MTK_KERNEL_HDR\n");
            flags[MTK_KERNEL] = true;
            k_hdr = reinterpret_cast<const mtk_hdr *>(kernel);
            fprintf(stderr, "%-*s [%u]\n", PADDING, "SIZE", k_hdr->size);
            fprintf(stderr, "%-*s [%s]\n", PADDING, "NAME", k_hdr->name);
            kernel += sizeof(mtk_hdr);
            hdr->kernel_size() -= sizeof(mtk_hdr);
            k_fmt = check_fmt_lg(kernel, hdr->kernel_size());
        }
        if (k_fmt == ZIMAGE) {
            z_hdr = reinterpret_cast<const zimage_hdr *>(kernel);
            if (const void *gzip = memmem(kernel, hdr->kernel_size(), GZIP1_MAGIC "\x08\x00", 4)) {
                fprintf(stderr, "ZIMAGE_KERNEL\n");
                z_info.hdr_sz = (const uint8_t *) gzip - kernel;

                // Find end of piggy
                uint32_t zImage_size = z_hdr->end - z_hdr->start;
                uint32_t piggy_end = zImage_size;
                uint32_t offsets[16];
                memcpy(offsets, kernel + zImage_size - sizeof(offsets), sizeof(offsets));
                for (int i = 15; i >= 0; --i) {
                    if (offsets[i] > (zImage_size - 0xFF) && offsets[i] < zImage_size) {
                        piggy_end = offsets[i];
                        break;
                    }
                }

                if (piggy_end == zImage_size) {
                    fprintf(stderr, "! Could not find end of zImage piggy, keeping raw kernel\n");
                } else {
                    flags[ZIMAGE_KERNEL] = true;
                    z_info.tail = byte_view(kernel + piggy_end, hdr->kernel_size() - piggy_end);
                    kernel += z_info.hdr_sz;
                    hdr->kernel_size() = piggy_end - z_info.hdr_sz;
                    k_fmt = check_fmt_lg(kernel, hdr->kernel_size());
                }
            } else {
                fprintf(stderr, "! Could not find zImage gzip piggy, keeping raw kernel\n");
            }
        }
        fprintf(stderr, "%-*s [%s]\n", PADDING, "KERNEL_FMT", fmt2name[k_fmt]);
    }
    if (auto size = hdr->ramdisk_size()) {
        if (hdr->is_vendor() && hdr->header_version() >= 4) {
            // v4 vendor boot contains multiple ramdisks
            // Do not try to mess with it for now
            r_fmt = UNKNOWN;
        } else {
            r_fmt = check_fmt_lg(ramdisk, size);
        }
        if (r_fmt == MTK) {
            fprintf(stderr, "MTK_RAMDISK_HDR\n");
            flags[MTK_RAMDISK] = true;
            r_hdr = reinterpret_cast<const mtk_hdr *>(ramdisk);
            fprintf(stderr, "%-*s [%u]\n", PADDING, "SIZE", r_hdr->size);
            fprintf(stderr, "%-*s [%s]\n", PADDING, "NAME", r_hdr->name);
            ramdisk += sizeof(mtk_hdr);
            hdr->ramdisk_size() -= sizeof(mtk_hdr);
            r_fmt = check_fmt_lg(ramdisk, hdr->ramdisk_size());
        }
        fprintf(stderr, "%-*s [%s]\n", PADDING, "RAMDISK_FMT", fmt2name[r_fmt]);
    }
    if (auto size = hdr->extra_size()) {
        e_fmt = check_fmt_lg(extra, size);
        fprintf(stderr, "%-*s [%s]\n", PADDING, "EXTRA_FMT", fmt2name[e_fmt]);
    }

    if (tail.sz()) {
        // Check special flags
        if (tail.sz() >= 16 && BUFFER_MATCH(tail.buf(), SEANDROID_MAGIC)) {
            fprintf(stderr, "SAMSUNG_SEANDROID\n");
            flags[SEANDROID_FLAG] = true;
        } else if (tail.sz() >= 16 && BUFFER_MATCH(tail.buf(), LG_BUMP_MAGIC)) {
            fprintf(stderr, "LG_BUMP_IMAGE\n");
            flags[LG_BUMP_FLAG] = true;
        }

        // Check if the image is signed
        if (verify()) {
            fprintf(stderr, "AVB1_SIGNED\n");
            flags[AVB1_SIGNED_FLAG] = true;
        }

        // Find AVB footer
        const void *footer = tail.buf() + tail.sz() - sizeof(AvbFooter);
        if (BUFFER_MATCH(footer, AVB_FOOTER_MAGIC)) {
            avb_footer = reinterpret_cast<const AvbFooter*>(footer);
            // Double check if meta header exists
            const void *meta = base_addr + __builtin_bswap64(avb_footer->vbmeta_offset);
            if (BUFFER_MATCH(meta, AVB_MAGIC)) {
                fprintf(stderr, "VBMETA\n");
                flags[AVB_FLAG] = true;
                vbmeta = reinterpret_cast<const AvbVBMetaImageHeader*>(meta);
            }
        }
    }

    this->hdr = hdr;
    return true;
}

bool boot_img::verify(const char *cert) const {
    return rust::verify_boot_image(*this, cert);
}

int split_image_dtb(const char *filename) {
    mmap_data img(filename);

    if (int off = find_dtb_offset(img.buf(), img.sz()); off > 0) {
        format_t fmt = check_fmt_lg(img.buf(), img.sz());
        if (COMPRESSED(fmt)) {
            int fd = creat(KERNEL_FILE, 0644);
            decompress(fmt, fd, img.buf(), off);
            close(fd);
        } else {
            dump(img.buf(), off, KERNEL_FILE);
        }
        dump(img.buf() + off, img.sz() - off, KER_DTB_FILE);
        return 0;
    } else {
        fprintf(stderr, "Cannot find DTB in %s\n", filename);
        return 1;
    }
}

int unpack(const char *image, bool skip_decomp, bool hdr) {
    const boot_img boot(image);

    if (hdr)
        boot.hdr->dump_hdr_file();

    // Dump kernel
    if (!skip_decomp && COMPRESSED(boot.k_fmt)) {
        if (boot.hdr->kernel_size() != 0) {
            int fd = creat(KERNEL_FILE, 0644);
            decompress(boot.k_fmt, fd, boot.kernel, boot.hdr->kernel_size());
            close(fd);
        }
    } else {
        dump(boot.kernel, boot.hdr->kernel_size(), KERNEL_FILE);
    }

    // Dump kernel_dtb
    dump(boot.kernel_dtb.buf(), boot.kernel_dtb.sz(), KER_DTB_FILE);

    // Dump ramdisk
    if (!skip_decomp && COMPRESSED(boot.r_fmt)) {
        if (boot.hdr->ramdisk_size() != 0) {
            int fd = creat(RAMDISK_FILE, 0644);
            decompress(boot.r_fmt, fd, boot.ramdisk, boot.hdr->ramdisk_size());
            close(fd);
        }
    } else {
        dump(boot.ramdisk, boot.hdr->ramdisk_size(), RAMDISK_FILE);
    }

    // Dump second
    dump(boot.second, boot.hdr->second_size(), SECOND_FILE);

    // Dump extra
    if (!skip_decomp && COMPRESSED(boot.e_fmt)) {
        if (boot.hdr->extra_size() != 0) {
            int fd = creat(EXTRA_FILE, 0644);
            decompress(boot.e_fmt, fd, boot.extra, boot.hdr->extra_size());
            close(fd);
        }
    } else {
        dump(boot.extra, boot.hdr->extra_size(), EXTRA_FILE);
    }

    // Dump recovery_dtbo
    dump(boot.recovery_dtbo, boot.hdr->recovery_dtbo_size(), RECV_DTBO_FILE);

    // Dump dtb
    dump(boot.dtb, boot.hdr->dtb_size(), DTB_FILE);

    return boot.flags[CHROMEOS_FLAG] ? 2 : 0;
}

#define file_align_with(page_size) \
write_zero(fd, align_padding(lseek(fd, 0, SEEK_CUR) - off.header, page_size))

#define file_align() file_align_with(boot.hdr->page_size())

void repack(const char *src_img, const char *out_img, bool skip_comp) {
    const boot_img boot(src_img);
    fprintf(stderr, "Repack to boot image: [%s]\n", out_img);

    struct {
        uint32_t header;
        uint32_t kernel;
        uint32_t ramdisk;
        uint32_t second;
        uint32_t extra;
        uint32_t dtb;
        uint32_t total;
        uint32_t vbmeta;
    } off{};

    // Create a new boot header and reset sizes
    auto hdr = boot.hdr->clone();
    hdr->kernel_size() = 0;
    hdr->ramdisk_size() = 0;
    hdr->second_size() = 0;
    hdr->dtb_size() = 0;

    if (access(HEADER_FILE, R_OK) == 0)
        hdr->load_hdr_file();

    /***************
     * Write blocks
     ***************/

    // Create new image
    int fd = creat(out_img, 0644);

    if (boot.flags[DHTB_FLAG]) {
        // Skip DHTB header
        write_zero(fd, sizeof(dhtb_hdr));
    } else if (boot.flags[BLOB_FLAG]) {
        xwrite(fd, boot.map.buf(), sizeof(blob_hdr));
    } else if (boot.flags[NOOKHD_FLAG]) {
        xwrite(fd, boot.map.buf(), NOOKHD_PRE_HEADER_SZ);
    } else if (boot.flags[ACCLAIM_FLAG]) {
        xwrite(fd, boot.map.buf(), ACCLAIM_PRE_HEADER_SZ);
    }

    // Copy raw header
    off.header = lseek(fd, 0, SEEK_CUR);
    xwrite(fd, boot.payload.buf(), hdr->hdr_space());

    // kernel
    off.kernel = lseek(fd, 0, SEEK_CUR);
    if (boot.flags[MTK_KERNEL]) {
        // Copy MTK headers
        xwrite(fd, boot.k_hdr, sizeof(mtk_hdr));
    }
    if (boot.flags[ZIMAGE_KERNEL]) {
        // Copy zImage headers
        xwrite(fd, boot.z_hdr, boot.z_info.hdr_sz);
    }
    if (access(KERNEL_FILE, R_OK) == 0) {
        mmap_data m(KERNEL_FILE);
        if (!skip_comp && !COMPRESSED_ANY(check_fmt(m.buf(), m.sz())) && COMPRESSED(boot.k_fmt)) {
            // Always use zopfli for zImage compression
            auto fmt = (boot.flags[ZIMAGE_KERNEL] && boot.k_fmt == GZIP) ? ZOPFLI : boot.k_fmt;
            hdr->kernel_size() = compress(fmt, fd, m.buf(), m.sz());
        } else {
            hdr->kernel_size() = xwrite(fd, m.buf(), m.sz());
        }

        if (boot.flags[ZIMAGE_KERNEL]) {
            if (hdr->kernel_size() > boot.hdr->kernel_size()) {
                fprintf(stderr, "! Recompressed kernel is too large, using original kernel\n");
                ftruncate64(fd, lseek64(fd, - (off64_t) hdr->kernel_size(), SEEK_CUR));
                xwrite(fd, boot.kernel, boot.hdr->kernel_size());
            } else if (!skip_comp) {
                // Pad zeros to make sure the zImage file size does not change
                // Also ensure the last 4 bytes are the uncompressed vmlinux size
                uint32_t sz = m.sz();
                write_zero(fd, boot.hdr->kernel_size() - hdr->kernel_size() - sizeof(sz));
                xwrite(fd, &sz, sizeof(sz));
            }

            // zImage size shall remain the same
            hdr->kernel_size() = boot.hdr->kernel_size();
        }
    } else if (boot.hdr->kernel_size() != 0) {
        xwrite(fd, boot.kernel, boot.hdr->kernel_size());
        hdr->kernel_size() = boot.hdr->kernel_size();
    }
    if (boot.flags[ZIMAGE_KERNEL]) {
        // Copy zImage tail and adjust size accordingly
        hdr->kernel_size() += boot.z_info.hdr_sz;
        hdr->kernel_size() += xwrite(fd, boot.z_info.tail.buf(), boot.z_info.tail.sz());
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
    if (access(RAMDISK_FILE, R_OK) == 0) {
        mmap_data m(RAMDISK_FILE);
        auto r_fmt = boot.r_fmt;
        if (!skip_comp && !hdr->is_vendor() && hdr->header_version() == 4 && r_fmt != LZ4_LEGACY) {
            // A v4 boot image ramdisk will have to be merged with other vendor ramdisks,
            // and they have to use the exact same compression method. v4 GKIs are required to
            // use lz4 (legacy), so hardcode the format here.
            fprintf(stderr, "RAMDISK_FMT: [%s] -> [%s]\n", fmt2name[r_fmt], fmt2name[LZ4_LEGACY]);
            r_fmt = LZ4_LEGACY;
        }
        if (!skip_comp && !COMPRESSED_ANY(check_fmt(m.buf(), m.sz())) && COMPRESSED(r_fmt)) {
            hdr->ramdisk_size() = compress(r_fmt, fd, m.buf(), m.sz());
        } else {
            hdr->ramdisk_size() = xwrite(fd, m.buf(), m.sz());
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
        if (!skip_comp && !COMPRESSED_ANY(check_fmt(m.buf(), m.sz())) && COMPRESSED(boot.e_fmt)) {
            hdr->extra_size() = compress(boot.e_fmt, fd, m.buf(), m.sz());
        } else {
            hdr->extra_size() = xwrite(fd, m.buf(), m.sz());
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

    // Directly copy ignored blobs
    if (boot.ignore.sz()) {
        // ignore.sz() should already be aligned
        xwrite(fd, boot.ignore.buf(), boot.ignore.sz());
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

    off.total = lseek(fd, 0, SEEK_CUR);
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
        if (current < boot.map.sz()) {
            write_zero(fd, boot.map.sz() - current);
        }
    }

    /******************
     * Patch the image
     ******************/

    // Map output image as rw
    mmap_data out(out_img, true);

    // MTK headers
    if (boot.flags[MTK_KERNEL]) {
        auto m_hdr = reinterpret_cast<mtk_hdr *>(out.buf() + off.kernel);
        m_hdr->size = hdr->kernel_size();
        hdr->kernel_size() += sizeof(mtk_hdr);
    }
    if (boot.flags[MTK_RAMDISK]) {
        auto m_hdr = reinterpret_cast<mtk_hdr *>(out.buf() + off.ramdisk);
        m_hdr->size = hdr->ramdisk_size();
        hdr->ramdisk_size() += sizeof(mtk_hdr);
    }

    // Make sure header size matches
    hdr->header_size() = hdr->hdr_size();

    // Update checksum
    if (char *id = hdr->id()) {
        auto ctx = get_sha(!boot.flags[SHA256_FLAG]);
        uint32_t size = hdr->kernel_size();
        ctx->update(byte_view(out.buf() + off.kernel, size));
        ctx->update(byte_view(&size, sizeof(size)));
        size = hdr->ramdisk_size();
        ctx->update(byte_view(out.buf() + off.ramdisk, size));
        ctx->update(byte_view(&size, sizeof(size)));
        size = hdr->second_size();
        ctx->update(byte_view(out.buf() + off.second, size));
        ctx->update(byte_view(&size, sizeof(size)));
        size = hdr->extra_size();
        if (size) {
            ctx->update(byte_view(out.buf() + off.extra, size));
            ctx->update(byte_view(&size, sizeof(size)));
        }
        uint32_t ver = hdr->header_version();
        if (ver == 1 || ver == 2) {
            size = hdr->recovery_dtbo_size();
            ctx->update(byte_view(out.buf() + hdr->recovery_dtbo_offset(), size));
            ctx->update(byte_view(&size, sizeof(size)));
        }
        if (ver == 2) {
            size = hdr->dtb_size();
            ctx->update(byte_view(out.buf() + off.dtb, size));
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
        memcpy(out.buf() + off.header + AMONET_MICROLOADER_SZ, hdr->raw_hdr(), real_hdr_sz);
    } else {
        memcpy(out.buf() + off.header, hdr->raw_hdr(), hdr->hdr_size());
    }

    if (boot.flags[AVB_FLAG]) {
        // Copy and patch AVB structures
        auto footer = reinterpret_cast<AvbFooter*>(out.buf() + out.sz() - sizeof(AvbFooter));
        memcpy(footer, boot.avb_footer, sizeof(AvbFooter));
        footer->original_image_size = __builtin_bswap64(off.total);
        footer->vbmeta_offset = __builtin_bswap64(off.vbmeta);
        if (check_env("PATCHVBMETAFLAG")) {
            auto vbmeta = reinterpret_cast<AvbVBMetaImageHeader*>(out.buf() + off.vbmeta);
            vbmeta->flags = __builtin_bswap32(3);
        }
    }

    if (boot.flags[DHTB_FLAG]) {
        // DHTB header
        auto d_hdr = reinterpret_cast<dhtb_hdr *>(out.buf());
        memcpy(d_hdr, DHTB_MAGIC, 8);
        d_hdr->size = off.total - sizeof(dhtb_hdr);
        sha256_hash(byte_view(out.buf() + sizeof(dhtb_hdr), d_hdr->size),
                    byte_data(d_hdr->checksum, 32));
    } else if (boot.flags[BLOB_FLAG]) {
        // Blob header
        auto b_hdr = reinterpret_cast<blob_hdr *>(out.buf());
        b_hdr->size = off.total - sizeof(blob_hdr);
    }

    // Sign the image after we finish patching the boot image
    if (boot.flags[AVB1_SIGNED_FLAG]) {
        byte_view payload(out.buf() + off.header, off.total - off.header);
        auto sig = rust::sign_boot_image(payload, "/boot", nullptr, nullptr);
        if (!sig.empty()) {
            lseek(fd, off.total, SEEK_SET);
            xwrite(fd, sig.data(), sig.size());
        }
    }

    close(fd);
}

int verify(const char *image, const char *cert) {
    const boot_img boot(image);
    if (cert == nullptr) {
        // Boot image parsing already checks if the image is signed
        return boot.flags[AVB1_SIGNED_FLAG] ? 0 : 1;
    } else {
        // Provide a custom certificate and re-verify
        return boot.verify(cert) ? 0 : 1;
    }
}

int sign(const char *image, const char *name, const char *cert, const char *key) {
    const boot_img boot(image);
    auto sig = rust::sign_boot_image(boot.payload, name, cert, key);
    if (sig.empty())
        return 1;

    auto eof = boot.tail.buf() - boot.map.buf();
    int fd = xopen(image, O_WRONLY | O_CLOEXEC);
    if (lseek(fd, eof, SEEK_SET) != eof || xwrite(fd, sig.data(), sig.size()) != sig.size()) {
        close(fd);
        return 1;
    }
    if (auto off = lseek(fd, 0, SEEK_CUR); off < boot.map.sz()) {
        // Wipe out rest of tail
        write_zero(fd, boot.map.sz() - off);
    }
    close(fd);
    return 0;
}
