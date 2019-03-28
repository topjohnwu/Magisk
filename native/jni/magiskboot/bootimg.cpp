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
#include <utils.h>
#include <logging.h>

#include "bootimg.h"
#include "magiskboot.h"
#include "compress.h"

using namespace std;

uint32_t dyn_img_hdr::j32 = 0;
uint64_t dyn_img_hdr::j64 = 0;

static int64_t one_step(unique_ptr<Compression> &&ptr, int fd, const void *in, size_t size) {
	ptr->set_out(make_unique<FDOutStream>(fd));
	if (!ptr->write(in, size))
		return -1;
	return ptr->finalize();
}

static int64_t decompress(format_t type, int fd, const void *in, size_t size) {
	return one_step(unique_ptr<Compression>(get_decoder(type)), fd, in, size);
}

static int64_t compress(format_t type, int fd, const void *in, size_t size) {
	return one_step(unique_ptr<Compression>(get_encoder(type)), fd, in, size);
}

static void dump(void *buf, size_t size, const char *filename) {
	if (size == 0)
		return;
	int fd = creat(filename, 0644);
	xwrite(fd, buf, size);
	close(fd);
}

static size_t restore(const char *filename, int fd) {
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

boot_img::~boot_img() {
	munmap(map_addr, map_size);
	delete k_hdr;
	delete r_hdr;
	delete b_hdr;
}

#define UNSUPP_RET  1
#define CHROME_RET  2
int boot_img::parse_file(const char *image) {
	mmap_ro(image, map_addr, map_size);
	fprintf(stderr, "Parsing boot image: [%s]\n", image);
	for (uint8_t *head = map_addr; head < map_addr + map_size; ++head) {
		switch (check_fmt(head, map_size)) {
		case CHROMEOS:
			// The caller should know it's chromeos, as it needs additional signing
			flags |= CHROMEOS_FLAG;
			break;
		case DHTB:
			flags |= DHTB_FLAG;
			flags |= SEANDROID_FLAG;
			fprintf(stderr, "DHTB_HDR\n");
			break;
		case BLOB:
			flags |= BLOB_FLAG;
			fprintf(stderr, "TEGRA_BLOB\n");
			b_hdr = new blob_hdr();
			memcpy(b_hdr, head, sizeof(blob_hdr));
			head += sizeof(blob_hdr) - 1;
			break;
		case AOSP:
			return parse_image(head);

		/* Unsupported */
		case ELF32:
		case ELF64:
			exit(UNSUPP_RET);
		default:
			break;
		}
	}
	exit(UNSUPP_RET);
}

#define pos_align() pos = do_align(pos, hdr.page_size())
int boot_img::parse_image(uint8_t *head) {
	auto hp = (boot_img_hdr*) head;
	if (hp->page_size >= 0x02000000) {
		fprintf(stderr, "PXA_BOOT_HDR\n");
		hdr.set_hdr(new boot_img_hdr_pxa());
		memcpy(*hdr, head, sizeof(boot_img_hdr_pxa));
	} else {
		if (memcmp(hp->cmdline, NOOKHD_MAGIC, 12) == 0 ||
			memcmp(hp->cmdline, NOOKHD_NEW_MAGIC, 26) == 0) {
			flags |= NOOKHD_FLAG;
			fprintf(stderr, "NOOKHD_GREEN_LOADER\n");
			head += NOOKHD_PRE_HEADER_SZ;
		} else if (memcmp(hp->name, ACCLAIM_MAGIC, 10) == 0) {
			flags |= ACCLAIM_FLAG;
			fprintf(stderr, "ACCLAIM_BAUWKSBOOT\n");
			head += ACCLAIM_PRE_HEADER_SZ;
		}
		hdr.set_hdr(new boot_img_hdr());
		memcpy(*hdr, head, sizeof(boot_img_hdr));
	}

	size_t pos = hdr.page_size();

	flags |= hdr.id()[SHA_DIGEST_SIZE] ? SHA256_FLAG : 0;

	print_hdr();

	kernel = head + pos;
	pos += hdr->kernel_size;
	pos_align();

	ramdisk = head + pos;
	pos += hdr->ramdisk_size;
	pos_align();

	second = head + pos;
	pos += hdr->second_size;
	pos_align();

	extra = head + pos;
	pos += hdr.extra_size();
	pos_align();

	recov_dtbo = head + pos;
	pos += hdr.recovery_dtbo_size();
	pos_align();

	dtb = head + pos;
	pos += hdr.dtb_size();
	pos_align();

	if (head + pos < map_addr + map_size) {
		tail = head + pos;
		tail_size = map_size - (tail - map_addr);
	}

	// Check tail info, currently only for LG Bump and Samsung SEANDROIDENFORCE
	if (tail_size >= 16 && memcmp(tail, SEANDROID_MAGIC, 16) == 0) {
		flags |= SEANDROID_FLAG;
	} else if (tail_size >= 16 && memcmp(tail, LG_BUMP_MAGIC, 16) == 0) {
		flags |= LG_BUMP_FLAG;
	}

	find_dtb();

	k_fmt = check_fmt(kernel, hdr->kernel_size);
	r_fmt = check_fmt(ramdisk, hdr->ramdisk_size);

	// Check MTK
	if (k_fmt == MTK) {
		fprintf(stderr, "MTK_KERNEL_HDR\n");
		flags |= MTK_KERNEL;
		k_hdr = new mtk_hdr();
		memcpy(k_hdr, kernel, sizeof(mtk_hdr));
		fprintf(stderr, "KERNEL          [%u]\n", k_hdr->size);
		fprintf(stderr, "NAME            [%s]\n", k_hdr->name);
		kernel += 512;
		hdr->kernel_size -= 512;
		k_fmt = check_fmt(kernel, hdr->kernel_size);
	}
	if (r_fmt == MTK) {
		fprintf(stderr, "MTK_RAMDISK_HDR\n");
		flags |= MTK_RAMDISK;
		r_hdr = new mtk_hdr();
		memcpy(r_hdr, ramdisk, sizeof(mtk_hdr));
		fprintf(stderr, "RAMDISK         [%u]\n", r_hdr->size);
		fprintf(stderr, "NAME            [%s]\n", r_hdr->name);
		ramdisk += 512;
		hdr->ramdisk_size -= 512;
		r_fmt = check_fmt(ramdisk, hdr->ramdisk_size);
	}

	fprintf(stderr, "KERNEL_FMT      [%s]\n", fmt2name[k_fmt]);
	fprintf(stderr, "RAMDISK_FMT     [%s]\n", fmt2name[r_fmt]);

	return (flags & CHROMEOS_FLAG) ? CHROME_RET : 0;
}

void boot_img::find_dtb() {
	for (uint32_t i = 0; i < hdr->kernel_size; ++i) {
		auto fdt_hdr = reinterpret_cast<fdt_header *>(kernel + i);
		if (fdt32_to_cpu(fdt_hdr->magic) != FDT_MAGIC)
			continue;

		// Check that fdt_header.totalsize does not overflow kernel image size
		uint32_t totalsize = fdt32_to_cpu(fdt_hdr->totalsize);
		if (totalsize > hdr->kernel_size - i) {
			fprintf(stderr, "Invalid DTB detection at 0x%x: size (%u) > remaining (%u)\n",
					i, totalsize, hdr->kernel_size - i);
			continue;
		}

		// Check that fdt_header.off_dt_struct does not overflow kernel image size
		uint32_t off_dt_struct = fdt32_to_cpu(fdt_hdr->off_dt_struct);
		if (off_dt_struct > hdr->kernel_size - i) {
			fprintf(stderr, "Invalid DTB detection at 0x%x: "
							"struct offset (%u) > remaining (%u)\n",
					i, off_dt_struct, hdr->kernel_size - i);
			continue;
		}

		// Check that fdt_node_header.tag of first node is FDT_BEGIN_NODE
		auto fdt_node_hdr = reinterpret_cast<fdt_node_header *>(kernel + i + off_dt_struct);
		if (fdt32_to_cpu(fdt_node_hdr->tag) != FDT_BEGIN_NODE) {
			fprintf(stderr, "Invalid DTB detection at 0x%x: "
							"header tag of first node != FDT_BEGIN_NODE\n", i);
			continue;
		}

		kernel_dtb = kernel + i;
		kernel_dt_size = hdr->kernel_size - i;
		hdr->kernel_size = i;
		fprintf(stderr, "KERNEL_DTB      [%u]\n", kernel_dt_size);
		break;
	}
}

void boot_img::print_hdr() {
	fprintf(stderr, "HEADER_VER      [%u]\n", hdr.header_version());
	fprintf(stderr, "KERNEL_SZ       [%u]\n", hdr->kernel_size);
	fprintf(stderr, "RAMDISK_SZ      [%u]\n", hdr->ramdisk_size);
	fprintf(stderr, "SECOND_SZ       [%u]\n", hdr->second_size);
	fprintf(stderr, "EXTRA_SZ        [%u]\n", hdr.extra_size());
	fprintf(stderr, "RECOV_DTBO_SZ   [%u]\n", hdr.recovery_dtbo_size());
	fprintf(stderr, "DTB             [%u]\n", hdr.dtb_size());

	uint32_t ver = hdr.os_version();
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

	fprintf(stderr, "PAGESIZE        [%u]\n", hdr.page_size());
	fprintf(stderr, "NAME            [%s]\n", hdr.name());
	fprintf(stderr, "CMDLINE         [%.512s%.1024s]\n", hdr.cmdline(), hdr.extra_cmdline());
	fprintf(stderr, "CHECKSUM        [");
	for (int i = 0; hdr.id()[i]; ++i)
		fprintf(stderr, "%02x", hdr.id()[i]);
	fprintf(stderr, "]\n");
}

int unpack(const char *image, bool hdr) {
	boot_img boot {};
	int ret = boot.parse_file(image);
	int fd;

	if (hdr) {
		FILE *fp = xfopen(HEADER_FILE, "w");
		fprintf(fp, "pagesize=%u\n", boot.hdr.page_size());
		fprintf(fp, "name=%s\n", boot.hdr.name());
		fprintf(fp, "cmdline=%.512s%.1024s\n", boot.hdr.cmdline(), boot.hdr.extra_cmdline());
		uint32_t ver = boot.hdr.os_version();
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

	// Dump kernel
	if (COMPRESSED(boot.k_fmt)) {
		fd = creat(KERNEL_FILE, 0644);
		decompress(boot.k_fmt, fd, boot.kernel, boot.hdr->kernel_size);
		close(fd);
	} else {
		dump(boot.kernel, boot.hdr->kernel_size, KERNEL_FILE);
	}

	// Dump dtb
	dump(boot.kernel_dtb, boot.kernel_dt_size, KER_DTB_FILE);

	// Dump ramdisk
	if (COMPRESSED(boot.r_fmt)) {
		fd = creat(RAMDISK_FILE, 0644);
		decompress(boot.r_fmt, fd, boot.ramdisk, boot.hdr->ramdisk_size);
		close(fd);
	} else {
		dump(boot.ramdisk, boot.hdr->ramdisk_size, RAMDISK_FILE);
	}

	// Dump second
	dump(boot.second, boot.hdr->second_size, SECOND_FILE);

	// Dump extra
	dump(boot.extra, boot.hdr.extra_size(), EXTRA_FILE);

	// Dump recovery_dtbo
	dump(boot.recov_dtbo, boot.hdr.recovery_dtbo_size(), RECV_DTBO_FILE);

	// Dump dtb
	dump(boot.dtb, boot.hdr.dtb_size(), DTB_FILE);
	return ret;
}

#define file_align() write_zero(fd, align_off(lseek(fd, 0, SEEK_CUR) - header_off, boot.hdr.page_size()))
void repack(const char* orig_image, const char* out_image) {
	boot_img boot {};

	off_t header_off, kernel_off, ramdisk_off, second_off, extra_off;

	// Parse original image
	boot.parse_file(orig_image);

	// Reset sizes
	boot.hdr->kernel_size = 0;
	boot.hdr->ramdisk_size = 0;
	boot.hdr->second_size = 0;
	boot.hdr.dtb_size() = 0;
	boot.kernel_dt_size = 0;

	fprintf(stderr, "Repack to boot image: [%s]\n", out_image);

	// Create new image
	int fd = creat(out_image, 0644);

	if (boot.flags & DHTB_FLAG) {
		// Skip DHTB header
		write_zero(fd, 512);
	} else if (boot.flags & BLOB_FLAG) {
		// Skip blob header
		write_zero(fd, sizeof(blob_hdr));
	} else if (boot.flags & NOOKHD_FLAG) {
		restore_buf(fd, boot.map_addr, NOOKHD_PRE_HEADER_SZ);
	} else if (boot.flags & ACCLAIM_FLAG) {
		restore_buf(fd, boot.map_addr, ACCLAIM_PRE_HEADER_SZ);
	}

	// header
	if (access(HEADER_FILE, R_OK) == 0) {
		parse_prop_file(HEADER_FILE, [&](string_view key, string_view value) -> bool {
			if (key == "page_size") {
				boot.hdr.page_size() = parse_int(value);
			} else if (key == "name") {
				memset(boot.hdr.name(), 0, 16);
				memcpy(boot.hdr.name(), value.data(), value.length() > 15 ? 15 : value.length());
			} else if (key == "cmdline") {
				memset(boot.hdr.cmdline(), 0, 512);
				memset(boot.hdr.extra_cmdline(), 0, 1024);
				if (value.length() > 512) {
					memcpy(boot.hdr.cmdline(), value.data(), 512);
					memcpy(boot.hdr.extra_cmdline(), &value[512], value.length() - 511);
				} else {
					memcpy(boot.hdr.cmdline(), value.data(), value.length());
				}
			} else if (key == "os_version") {
				int patch_level = boot.hdr.os_version() & 0x7ff;
				int a, b, c;
				sscanf(value.data(), "%d.%d.%d", &a, &b, &c);
				boot.hdr.os_version() = (((a << 14) | (b << 7) | c) << 11) | patch_level;
			} else if (key == "os_patch_level") {
				int os_version = boot.hdr.os_version() >> 11;
				int y, m;
				sscanf(value.data(), "%d-%d", &y, &m);
				y -= 2000;
				boot.hdr.os_version() = (os_version << 11) | (y << 4) | m;
			}
			return true;
		});
	}

	// Skip a page for header
	header_off = lseek(fd, 0, SEEK_CUR);
	write_zero(fd, boot.hdr.page_size());

	// kernel
	kernel_off = lseek(fd, 0, SEEK_CUR);
	if (boot.flags & MTK_KERNEL) {
		// Skip MTK header
		write_zero(fd, 512);
	}
	if (access(KERNEL_FILE, R_OK) == 0) {
		size_t raw_size;
		void *raw_buf;
		mmap_ro(KERNEL_FILE, raw_buf, raw_size);
		if (!COMPRESSED(check_fmt(raw_buf, raw_size)) && COMPRESSED(boot.k_fmt)) {
			boot.hdr->kernel_size = compress(boot.k_fmt, fd, raw_buf, raw_size);
		} else {
			boot.hdr->kernel_size = write(fd, raw_buf, raw_size);
		}
		munmap(raw_buf, raw_size);
	}

	// kernel dtb
	if (access(KER_DTB_FILE, R_OK) == 0)
		boot.hdr->kernel_size += restore(KER_DTB_FILE, fd);
	file_align();

	// ramdisk
	ramdisk_off = lseek(fd, 0, SEEK_CUR);
	if (boot.flags & MTK_RAMDISK) {
		// Skip MTK header
		write_zero(fd, 512);
	}
	if (access(RAMDISK_FILE, R_OK) == 0) {
		size_t raw_size;
		void *raw_buf;
		mmap_ro(RAMDISK_FILE, raw_buf, raw_size);
		if (!COMPRESSED(check_fmt(raw_buf, raw_size)) && COMPRESSED(boot.r_fmt)) {
			boot.hdr->ramdisk_size = compress(boot.r_fmt, fd, raw_buf, raw_size);
		} else {
			boot.hdr->ramdisk_size = write(fd, raw_buf, raw_size);
		}
		munmap(raw_buf, raw_size);
		file_align();
	}

	// second
	second_off = lseek(fd, 0, SEEK_CUR);
	if (access(SECOND_FILE, R_OK) == 0) {
		boot.hdr->second_size = restore(SECOND_FILE, fd);
		file_align();
	}

	// extra
	extra_off = lseek(fd, 0, SEEK_CUR);
	if (access(EXTRA_FILE, R_OK) == 0) {
		boot.hdr.extra_size() = restore(EXTRA_FILE, fd);
		file_align();
	}

	// recovery_dtbo
	if (access(RECV_DTBO_FILE, R_OK) == 0) {
		boot.hdr.recovery_dtbo_offset() = lseek(fd, 0, SEEK_CUR);
		boot.hdr.recovery_dtbo_size() = restore(RECV_DTBO_FILE, fd);
		file_align();
	}

	// dtb
	if (access(DTB_FILE, R_OK) == 0) {
		boot.hdr.dtb_size() = restore(DTB_FILE, fd);
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

	// Map output image as rw
	munmap(boot.map_addr, boot.map_size);
	mmap_rw(out_image, boot.map_addr, boot.map_size);

	// MTK headers
	if (boot.flags & MTK_KERNEL) {
		boot.k_hdr->size = boot.hdr->kernel_size;
		boot.hdr->kernel_size += 512;
		memcpy(boot.map_addr + kernel_off, boot.k_hdr, sizeof(mtk_hdr));
	}
	if (boot.flags & MTK_RAMDISK) {
		boot.r_hdr->size = boot.hdr->ramdisk_size;
		boot.hdr->ramdisk_size += 512;
		memcpy(boot.map_addr + ramdisk_off, boot.r_hdr, sizeof(mtk_hdr));
	}

	// Update checksum
	HASH_CTX ctx;
	(boot.flags & SHA256_FLAG) ? SHA256_init(&ctx) : SHA_init(&ctx);
	uint32_t size = boot.hdr->kernel_size;
	HASH_update(&ctx, boot.map_addr + kernel_off, size);
	HASH_update(&ctx, &size, sizeof(size));
	size = boot.hdr->ramdisk_size;
	HASH_update(&ctx, boot.map_addr + ramdisk_off, size);
	HASH_update(&ctx, &size, sizeof(size));
	size = boot.hdr->second_size;
	HASH_update(&ctx, boot.map_addr + second_off, size);
	HASH_update(&ctx, &size, sizeof(size));
	size = boot.hdr.extra_size();
	if (size) {
		HASH_update(&ctx, boot.map_addr + extra_off, size);
		HASH_update(&ctx, &size, sizeof(size));
	}
	if (boot.hdr.header_version()) {
		size = boot.hdr.recovery_dtbo_size();
		HASH_update(&ctx, boot.map_addr + boot.hdr.recovery_dtbo_offset(), size);
		HASH_update(&ctx, &size, sizeof(size));
	}
	memset(boot.hdr.id(), 0, 32);
	memcpy(boot.hdr.id(), HASH_final(&ctx),
		   (boot.flags & SHA256_FLAG) ? SHA256_DIGEST_SIZE : SHA_DIGEST_SIZE);

	// Print new image info
	boot.print_hdr();

	// Try to fix the header
	if (boot.hdr.header_version() && boot.hdr.header_size() == 0)
		boot.hdr.header_size() = sizeof(boot_img_hdr);

	// Main header
	memcpy(boot.map_addr + header_off, *boot.hdr, boot.hdr.hdr_size());

	if (boot.flags & DHTB_FLAG) {
		// DHTB header
		dhtb_hdr *hdr = reinterpret_cast<dhtb_hdr *>(boot.map_addr);
		memcpy(hdr, DHTB_MAGIC, 8);
		hdr->size = boot.map_size - 512;
		SHA256_hash(boot.map_addr + 512, hdr->size, hdr->checksum);
	} else if (boot.flags & BLOB_FLAG) {
		// Blob headers
		boot.b_hdr->size = boot.map_size - sizeof(blob_hdr);
		memcpy(boot.map_addr, boot.b_hdr, sizeof(blob_hdr));
	}
}
