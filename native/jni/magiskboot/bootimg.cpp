#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libfdt.h>
#include <functional>
#include <memory>

#include <mincrypt/sha.h>
#include <mincrypt/sha256.h>
#include <utils.hpp>
#include <logging.hpp>

#include "bootimg.hpp"
#include "magiskboot.hpp"
#include "compress.hpp"

using namespace std;

uint32_t dyn_img_hdr::j32 = 0;
uint64_t dyn_img_hdr::j64 = 0;

static void decompress(format_t type, int fd, const void *in, size_t size) {
	auto ptr = get_decoder(type, make_unique<fd_stream>(fd));
	ptr->write(in, size);
}

static off_t compress(format_t type, int fd, const void *in, size_t size) {
	auto prev = lseek(fd, 0, SEEK_CUR);
	{
		auto strm = get_encoder(type, make_unique<fd_stream>(fd));
		strm->write(in, size);
	}
	auto now = lseek(fd, 0, SEEK_CUR);
	return now - prev;
}

static void dump(void *buf, size_t size, const char *filename) {
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

static void restore_buf(int fd, const void *buf, size_t size) {
	xwrite(fd, buf, size);
}

void dyn_img_hdr::print() {
	uint32_t ver = header_version();
	fprintf(stderr, "HEADER_VER      [%u]\n", ver);
	fprintf(stderr, "KERNEL_SZ       [%u]\n", kernel_size());
	fprintf(stderr, "RAMDISK_SZ      [%u]\n", ramdisk_size());
	fprintf(stderr, "SECOND_SZ       [%u]\n", second_size());
	if (ver == 0)
		fprintf(stderr, "EXTRA_SZ        [%u]\n", extra_size());
	if (ver >= 1)
		fprintf(stderr, "RECOV_DTBO_SZ   [%u]\n", recovery_dtbo_size());
	if (ver >= 2)
		fprintf(stderr, "DTB_SZ          [%u]\n", dtb_size());

	ver = os_version();
	if (ver) {
		int a,b,c,y,m = 0;
		int version, patch_level;
		version = ver >> 11;
		patch_level = ver & 0x7ff;

		a = (version >> 14) & 0x7f;
		b = (version >> 7) & 0x7f;
		c = version & 0x7f;
		fprintf(stderr, "OS_VERSION      [%d.%d.%d]\n", a, b, c);

		y = (patch_level >> 4) + 2000;
		m = patch_level & 0xf;
		fprintf(stderr, "OS_PATCH_LEVEL  [%d-%02d]\n", y, m);
	}

	fprintf(stderr, "PAGESIZE        [%u]\n", page_size());
	fprintf(stderr, "NAME            [%s]\n", name());
	fprintf(stderr, "CMDLINE         [%.512s%.1024s]\n", cmdline(), extra_cmdline());
	fprintf(stderr, "CHECKSUM        [");
	for (int i = 0; i < SHA256_DIGEST_SIZE; ++i)
		fprintf(stderr, "%02hhx", static_cast<uint8_t>(id()[i]));
	fprintf(stderr, "]\n");
}

void dyn_img_hdr::dump_hdr_file() {
	FILE *fp = xfopen(HEADER_FILE, "w");
	fprintf(fp, "pagesize=%u\n", page_size());
	fprintf(fp, "name=%s\n", name());
	fprintf(fp, "cmdline=%.512s%.1024s\n", cmdline(), extra_cmdline());
	uint32_t ver = os_version();
	if (ver) {
		int a, b, c, y, m = 0;
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
	parse_prop_file(HEADER_FILE, [=](string_view key, string_view value) -> bool {
		if (key == "page_size") {
			page_size() = parse_int(value);
		} else if (key == "name") {
			memset(name(), 0, 16);
			memcpy(name(), value.data(), value.length() > 15 ? 15 : value.length());
		} else if (key == "cmdline") {
			memset(cmdline(), 0, 512);
			memset(extra_cmdline(), 0, 1024);
			if (value.length() > 512) {
				memcpy(cmdline(), value.data(), 512);
				memcpy(extra_cmdline(), &value[512], value.length() - 511);
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

boot_img::boot_img(const char *image) {
	mmap_ro(image, map_addr, map_size);
	fprintf(stderr, "Parsing boot image: [%s]\n", image);
	for (uint8_t *addr = map_addr; addr < map_addr + map_size; ++addr) {
		switch (check_fmt(addr, map_size)) {
		case CHROMEOS:
			// chromeos require external signing
			flags |= CHROMEOS_FLAG;
			addr += 65535;
			break;
		case DHTB:
			flags |= (DHTB_FLAG | SEANDROID_FLAG);
			fprintf(stderr, "DHTB_HDR\n");
			addr += sizeof(dhtb_hdr) - 1;
			break;
		case BLOB:
			flags |= BLOB_FLAG;
			fprintf(stderr, "TEGRA_BLOB\n");
			addr += sizeof(blob_hdr) - 1;
			break;
		case AOSP:
			parse_image(addr);
			return;
		default:
			break;
		}
	}
	exit(1);
}

boot_img::~boot_img() {
	munmap(map_addr, map_size);
	delete hdr;
}

#define get_block(name) {\
name = addr + off; \
off += hdr->name##_size(); \
off = do_align(off, hdr->page_size()); \
}

void boot_img::parse_image(uint8_t *addr) {
	auto hp = reinterpret_cast<boot_img_hdr*>(addr);
	if (hp->page_size >= 0x02000000) {
		fprintf(stderr, "PXA_BOOT_HDR\n");
		hdr = new dyn_img_pxa(addr);
	} else {
		if (memcmp(hp->cmdline, NOOKHD_RL_MAGIC, 10) == 0 ||
			memcmp(hp->cmdline, NOOKHD_GL_MAGIC, 12) == 0 ||
			memcmp(hp->cmdline, NOOKHD_GR_MAGIC, 14) == 0 ||
			memcmp(hp->cmdline, NOOKHD_EB_MAGIC, 26) == 0 ||
			memcmp(hp->cmdline, NOOKHD_ER_MAGIC, 30) == 0) {
			flags |= NOOKHD_FLAG;
			fprintf(stderr, "NOOKHD_LOADER\n");
			addr += NOOKHD_PRE_HEADER_SZ;
		} else if (memcmp(hp->name, ACCLAIM_MAGIC, 10) == 0) {
			flags |= ACCLAIM_FLAG;
			fprintf(stderr, "ACCLAIM_LOADER\n");
			addr += ACCLAIM_PRE_HEADER_SZ;
		}

		if (hp->header_version == 1)
			hdr = new dyn_img_v1(addr);
		else if (hp->header_version == 2)
			hdr = new dyn_img_v2(addr);
		else
			hdr = new dyn_img_v0(addr);
	}

	for (int i = SHA_DIGEST_SIZE + 4; i < SHA256_DIGEST_SIZE; ++i) {
		if (hdr->id()[i]) {
			flags |= SHA256_FLAG;
			break;
		}
	}

	hdr->print();

	size_t off = hdr->page_size();
	hdr_addr = addr;
	get_block(kernel);
	get_block(ramdisk);
	get_block(second);
	get_block(extra);
	get_block(recovery_dtbo);
	get_block(dtb);

	if (addr + off < map_addr + map_size) {
		tail = addr + off;
		tail_size = map_size - (tail - map_addr);
	}

	// Check tail info, currently only for LG Bump and Samsung SEANDROIDENFORCE
	if (tail_size >= 16 && memcmp(tail, SEANDROID_MAGIC, 16) == 0) {
		flags |= SEANDROID_FLAG;
	} else if (tail_size >= 16 && memcmp(tail, LG_BUMP_MAGIC, 16) == 0) {
		flags |= LG_BUMP_FLAG;
	}

	find_kernel_dtb();

	k_fmt = check_fmt(kernel, hdr->kernel_size());
	r_fmt = check_fmt(ramdisk, hdr->ramdisk_size());

	// Check MTK
	if (k_fmt == MTK) {
		fprintf(stderr, "MTK_KERNEL_HDR\n");
		flags |= MTK_KERNEL;
		k_hdr = reinterpret_cast<mtk_hdr *>(kernel);
		fprintf(stderr, "KERNEL          [%u]\n", k_hdr->size);
		fprintf(stderr, "NAME            [%s]\n", k_hdr->name);
		kernel += sizeof(mtk_hdr);
		hdr->kernel_size() -= sizeof(mtk_hdr);
		k_fmt = check_fmt(kernel, hdr->kernel_size());
	}
	if (r_fmt == MTK) {
		fprintf(stderr, "MTK_RAMDISK_HDR\n");
		flags |= MTK_RAMDISK;
		r_hdr = reinterpret_cast<mtk_hdr *>(ramdisk);
		fprintf(stderr, "RAMDISK         [%u]\n", r_hdr->size);
		fprintf(stderr, "NAME            [%s]\n", r_hdr->name);
		ramdisk += sizeof(mtk_hdr);
		hdr->ramdisk_size() -= sizeof(mtk_hdr);
		r_fmt = check_fmt(ramdisk, hdr->ramdisk_size());
	}

	fprintf(stderr, "KERNEL_FMT      [%s]\n", fmt2name[k_fmt]);
	fprintf(stderr, "RAMDISK_FMT     [%s]\n", fmt2name[r_fmt]);
}

static int find_dtb_offset(uint8_t *buf, int sz) {
	for (int off = 0; off < sz - (int) sizeof(fdt_header); ++off) {
		auto fdt_hdr = reinterpret_cast<fdt_header *>(buf + off);
		if (fdt32_to_cpu(fdt_hdr->magic) != FDT_MAGIC)
			continue;

		// Check that fdt_header.totalsize does not overflow kernel image size
		uint32_t totalsize = fdt32_to_cpu(fdt_hdr->totalsize);
		if (totalsize + off > sz)
			continue;

		// Check that fdt_header.off_dt_struct does not overflow kernel image size
		uint32_t off_dt_struct = fdt32_to_cpu(fdt_hdr->off_dt_struct);
		if (off_dt_struct + off > sz)
			continue;

		// Check that fdt_node_header.tag of first node is FDT_BEGIN_NODE
		auto fdt_node_hdr = reinterpret_cast<fdt_node_header *>(buf + off + off_dt_struct);
		if (fdt32_to_cpu(fdt_node_hdr->tag) != FDT_BEGIN_NODE)
			continue;

		return off;
	}
	return -1;
}

void boot_img::find_kernel_dtb() {
	if (int off = find_dtb_offset(kernel, hdr->kernel_size()); off > 0) {
		kernel_dtb = kernel + off;
		kernel_dt_size = hdr->kernel_size() - off;
		hdr->kernel_size() = off;
		fprintf(stderr, "KERNEL_DTB      [%u]\n", kernel_dt_size);
	}
}

int split_image_dtb(const char *filename) {
	uint8_t *buf;
	size_t sz;
	mmap_ro(filename, buf, sz);
	run_finally f([=]{ munmap(buf, sz); });

	if (int off = find_dtb_offset(buf, sz); off > 0) {
		format_t fmt = check_fmt(buf, sz);
		if (COMPRESSED(fmt)) {
			int fd = creat(KERNEL_FILE, 0644);
			decompress(fmt, fd, buf, off);
			close(fd);
		} else {
			dump(buf, off, KERNEL_FILE);
		}
		dump(buf + off, sz - off, KER_DTB_FILE);
		return 0;
	} else {
		fprintf(stderr, "Cannot find DTB in %s\n", filename);
		return 1;
	}
}

int unpack(const char *image, bool nodecomp, bool hdr) {
	boot_img boot(image);

	if (hdr)
		boot.hdr->dump_hdr_file();

	// Dump kernel
	if (!nodecomp && COMPRESSED(boot.k_fmt)) {
		int fd = creat(KERNEL_FILE, 0644);
		decompress(boot.k_fmt, fd, boot.kernel, boot.hdr->kernel_size());
		close(fd);
	} else {
		dump(boot.kernel, boot.hdr->kernel_size(), KERNEL_FILE);
	}

	// Dump kernel_dtb
	dump(boot.kernel_dtb, boot.kernel_dt_size, KER_DTB_FILE);

	// Dump ramdisk
	if (!nodecomp && COMPRESSED(boot.r_fmt)) {
		int fd = creat(RAMDISK_FILE, 0644);
		decompress(boot.r_fmt, fd, boot.ramdisk, boot.hdr->ramdisk_size());
		close(fd);
	} else {
		dump(boot.ramdisk, boot.hdr->ramdisk_size(), RAMDISK_FILE);
	}

	// Dump second
	dump(boot.second, boot.hdr->second_size(), SECOND_FILE);

	// Dump extra
	dump(boot.extra, boot.hdr->extra_size(), EXTRA_FILE);

	// Dump recovery_dtbo
	dump(boot.recovery_dtbo, boot.hdr->recovery_dtbo_size(), RECV_DTBO_FILE);

	// Dump dtb
	dump(boot.dtb, boot.hdr->dtb_size(), DTB_FILE);

	return (boot.flags & CHROMEOS_FLAG) ? 2 : 0;
}

#define file_align() \
write_zero(fd, align_off(lseek(fd, 0, SEEK_CUR) - off.header, boot.hdr->page_size()))

void repack(const char* src_img, const char* out_img, bool nocomp) {
	boot_img boot(src_img);

	struct {
		uint32_t header;
		uint32_t kernel;
		uint32_t ramdisk;
		uint32_t second;
		uint32_t extra;
		uint32_t dtb;
	} off;

	fprintf(stderr, "Repack to boot image: [%s]\n", out_img);

	// Reset sizes
	boot.hdr->kernel_size() = 0;
	boot.hdr->ramdisk_size() = 0;
	boot.hdr->second_size() = 0;
	boot.hdr->dtb_size() = 0;
	boot.kernel_dt_size = 0;

	if (access(HEADER_FILE, R_OK) == 0)
		boot.hdr->load_hdr_file();

	/*****************
	 * Writing blocks
	 *****************/

	// Create new image
	int fd = creat(out_img, 0644);

	if (boot.flags & DHTB_FLAG) {
		// Skip DHTB header
		write_zero(fd, sizeof(dhtb_hdr));
	} else if (boot.flags & BLOB_FLAG) {
		restore_buf(fd, boot.map_addr, sizeof(blob_hdr));
	} else if (boot.flags & NOOKHD_FLAG) {
		restore_buf(fd, boot.map_addr, NOOKHD_PRE_HEADER_SZ);
	} else if (boot.flags & ACCLAIM_FLAG) {
		restore_buf(fd, boot.map_addr, ACCLAIM_PRE_HEADER_SZ);
	}

	// Copy a page for header
	off.header = lseek(fd, 0, SEEK_CUR);
	restore_buf(fd, boot.hdr_addr, boot.hdr->page_size());

	// kernel
	off.kernel = lseek(fd, 0, SEEK_CUR);
	if (boot.flags & MTK_KERNEL) {
		// Copy MTK headers
		restore_buf(fd, boot.k_hdr, sizeof(mtk_hdr));
	}
	if (access(KERNEL_FILE, R_OK) == 0) {
		size_t raw_size;
		void *raw_buf;
		mmap_ro(KERNEL_FILE, raw_buf, raw_size);
		if (!COMPRESSED_ANY(check_fmt(raw_buf, raw_size)) && COMPRESSED(boot.k_fmt)) {
			boot.hdr->kernel_size() = compress(boot.k_fmt, fd, raw_buf, raw_size);
		} else {
			boot.hdr->kernel_size() = xwrite(fd, raw_buf, raw_size);
		}
		munmap(raw_buf, raw_size);
	}

	// kernel dtb
	if (access(KER_DTB_FILE, R_OK) == 0)
		boot.hdr->kernel_size() += restore(fd, KER_DTB_FILE);
	file_align();

	// ramdisk
	off.ramdisk = lseek(fd, 0, SEEK_CUR);
	if (boot.flags & MTK_RAMDISK) {
		// Copy MTK headers
		restore_buf(fd, boot.r_hdr, sizeof(mtk_hdr));
	}
	if (access(RAMDISK_FILE, R_OK) == 0) {
		size_t raw_size;
		void *raw_buf;
		mmap_ro(RAMDISK_FILE, raw_buf, raw_size);
		if (!nocomp && !COMPRESSED_ANY(check_fmt(raw_buf, raw_size)) && COMPRESSED(boot.r_fmt)) {
			boot.hdr->ramdisk_size() = compress(boot.r_fmt, fd, raw_buf, raw_size);
		} else {
			boot.hdr->ramdisk_size() = xwrite(fd, raw_buf, raw_size);
		}
		munmap(raw_buf, raw_size);
		file_align();
	}

	// second
	off.second = lseek(fd, 0, SEEK_CUR);
	if (access(SECOND_FILE, R_OK) == 0) {
		boot.hdr->second_size() = restore(fd, SECOND_FILE);
		file_align();
	}

	// extra
	off.extra = lseek(fd, 0, SEEK_CUR);
	if (access(EXTRA_FILE, R_OK) == 0) {
		boot.hdr->extra_size() = restore(fd, EXTRA_FILE);
		file_align();
	}

	// recovery_dtbo
	if (access(RECV_DTBO_FILE, R_OK) == 0) {
		boot.hdr->recovery_dtbo_offset() = lseek(fd, 0, SEEK_CUR);
		boot.hdr->recovery_dtbo_size() = restore(fd, RECV_DTBO_FILE);
		file_align();
	}

	// dtb
	off.dtb = lseek(fd, 0, SEEK_CUR);
	if (access(DTB_FILE, R_OK) == 0) {
		boot.hdr->dtb_size() = restore(fd, DTB_FILE);
		file_align();
	}

	// Append tail info
	if (boot.flags & SEANDROID_FLAG) {
		restore_buf(fd, SEANDROID_MAGIC "\xFF\xFF\xFF\xFF", 20);
	}
	if (boot.flags & LG_BUMP_FLAG) {
		restore_buf(fd, LG_BUMP_MAGIC, 16);
	}

	close(fd);

	/*********************
	 * Patching the image
	 *********************/

	// Map output image as rw
	munmap(boot.map_addr, boot.map_size);
	mmap_rw(out_img, boot.map_addr, boot.map_size);

	// MTK headers
	if (boot.flags & MTK_KERNEL) {
		auto hdr = reinterpret_cast<mtk_hdr *>(boot.map_addr + off.kernel);
		hdr->size = boot.hdr->kernel_size();
		boot.hdr->kernel_size() += sizeof(*hdr);
	}
	if (boot.flags & MTK_RAMDISK) {
		auto hdr = reinterpret_cast<mtk_hdr *>(boot.map_addr + off.ramdisk);
		hdr->size = boot.hdr->ramdisk_size();
		boot.hdr->ramdisk_size() += sizeof(*hdr);
	}

	// Update checksum
	HASH_CTX ctx;
	(boot.flags & SHA256_FLAG) ? SHA256_init(&ctx) : SHA_init(&ctx);
	uint32_t size = boot.hdr->kernel_size();
	HASH_update(&ctx, boot.map_addr + off.kernel, size);
	HASH_update(&ctx, &size, sizeof(size));
	size = boot.hdr->ramdisk_size();
	HASH_update(&ctx, boot.map_addr + off.ramdisk, size);
	HASH_update(&ctx, &size, sizeof(size));
	size = boot.hdr->second_size();
	HASH_update(&ctx, boot.map_addr + off.second, size);
	HASH_update(&ctx, &size, sizeof(size));
	size = boot.hdr->extra_size();
	if (size) {
		HASH_update(&ctx, boot.map_addr + off.extra, size);
		HASH_update(&ctx, &size, sizeof(size));
	}
	if (boot.hdr->header_version() >= 1) {
		size = boot.hdr->recovery_dtbo_size();
		HASH_update(&ctx, boot.map_addr + boot.hdr->recovery_dtbo_offset(), size);
		HASH_update(&ctx, &size, sizeof(size));
	}
	if (boot.hdr->header_version() >= 2) {
		size = boot.hdr->dtb_size();
		HASH_update(&ctx, boot.map_addr + off.dtb, size);
		HASH_update(&ctx, &size, sizeof(size));
	}
	memset(boot.hdr->id(), 0, 32);
	memcpy(boot.hdr->id(), HASH_final(&ctx),
		   (boot.flags & SHA256_FLAG) ? SHA256_DIGEST_SIZE : SHA_DIGEST_SIZE);
	boot.hdr->header_size() = boot.hdr->hdr_size();

	// Print new image info
	boot.hdr->print();

	// Main header
	memcpy(boot.map_addr + off.header, **boot.hdr, boot.hdr->hdr_size());

	if (boot.flags & DHTB_FLAG) {
		// DHTB header
		auto hdr = reinterpret_cast<dhtb_hdr *>(boot.map_addr);
		memcpy(hdr, DHTB_MAGIC, 8);
		hdr->size = boot.map_size - sizeof(dhtb_hdr);
		SHA256_hash(boot.map_addr + sizeof(dhtb_hdr), hdr->size, hdr->checksum);
	} else if (boot.flags & BLOB_FLAG) {
		// Blob header
		auto hdr = reinterpret_cast<blob_hdr *>(boot.map_addr);
		hdr->size = boot.map_size - sizeof(blob_hdr);
	}
}
