#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libfdt.h>
#include <sys/mman.h>

#include <mincrypt/sha.h>
#include <mincrypt/sha256.h>

#include "bootimg.h"
#include "magiskboot.h"
#include "utils.h"
#include "logging.h"

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
	xsendfile(fd, ifd, NULL, size);
	close(ifd);
	return size;
}

static void restore_buf(int fd, const void *buf, size_t size) {
	xwrite(fd, buf, size);
}

boot_img::~boot_img() {
	munmap(map_addr, map_size);
	delete hdr;
	delete k_hdr;
	delete r_hdr;
	delete b_hdr;
}

#define CHROMEOS_RET       2
#define ELF32_RET          3
#define ELF64_RET          4
#define pos_align() pos = align(pos, page_size())

int boot_img::parse_image(const char * image) {
	mmap_ro(image, (void **) &map_addr, &map_size);

	// Parse image
	fprintf(stderr, "Parsing boot image: [%s]\n", image);
	for (uint8_t *head = map_addr; head < map_addr + map_size; ++head) {
		size_t pos = 0;

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
		case ELF32:
			exit(ELF32_RET);
		case ELF64:
			exit(ELF64_RET);
		case BLOB:
			flags |= BLOB_FLAG;
			fprintf(stderr, "TEGRA_BLOB\n");
			b_hdr = new blob_hdr();
			memcpy(b_hdr, head, sizeof(blob_hdr));
			break;
		case AOSP:
			// Read the header
			if (((boot_img_hdr*) head)->page_size >= 0x02000000) {
				flags |= PXA_FLAG;
				fprintf(stderr, "PXA_BOOT_HDR\n");
				hdr = new boot_img_hdr_pxa();
				memcpy(hdr, head, sizeof(boot_img_hdr_pxa));
			} else if (memcmp(((boot_img_hdr*) head)->cmdline, NOOKHD_MAGIC, 12) == 0
					   || memcmp(((boot_img_hdr*) head)->cmdline, NOOKHD_NEW_MAGIC, 26) == 0) {
				flags |= NOOKHD_FLAG;
				fprintf(stderr, "NOOKHD_GREEN_LOADER\n");
				head += NOOKHD_PRE_HEADER_SZ - 1;
				continue;
			} else if (memcmp(((boot_img_hdr*) head)->name, ACCLAIM_MAGIC, 10) == 0) {
				flags |= ACCLAIM_FLAG;
				fprintf(stderr, "ACCLAIM_BAUWKSBOOT\n");
				head += ACCLAIM_PRE_HEADER_SZ - 1;
				continue;
			} else {
				hdr = new boot_img_hdr();
				memcpy(hdr, head, sizeof(boot_img_hdr));
			}
			pos += page_size();

			flags |= id()[SHA_DIGEST_SIZE] ? SHA256_FLAG : 0;

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
			pos += extra_size();
			pos_align();

			recov_dtbo = head + pos;
			pos += recovery_dtbo_size();
			pos_align();

			if (pos < map_size) {
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

			char fmt[16];
			get_fmt_name(k_fmt, fmt);
			fprintf(stderr, "KERNEL_FMT      [%s]\n", fmt);
			get_fmt_name(r_fmt, fmt);
			fprintf(stderr, "RAMDISK_FMT     [%s]\n", fmt);

			return flags & CHROMEOS_FLAG ? CHROMEOS_RET : 0;
		default:
			break;
		}
	}
	LOGE("No boot image magic found!\n");
	exit(1);
}

void boot_img::find_dtb() {
	for (uint32_t i = 0; i < hdr->kernel_size; ++i) {
		if (memcmp(kernel + i, DTB_MAGIC, 4))
			continue;
		// Check that fdt_header.totalsize does not overflow kernel image size
		uint32_t dt_sz = fdt32_to_cpu(*(uint32_t *)(kernel + i + 4));
		if (dt_sz > hdr->kernel_size - i) {
			fprintf(stderr, "Invalid DTB detection at 0x%x: size (%u) > remaining (%u)\n",
					i, dt_sz, hdr->kernel_size - i);
			continue;
		}

		// Check that fdt_header.off_dt_struct does not overflow kernel image size
		uint32_t dt_struct_offset = fdt32_to_cpu(*(uint32_t *)(kernel + i + 8));
		if (dt_struct_offset > hdr->kernel_size - i) {
			fprintf(stderr, "Invalid DTB detection at 0x%x: "
							"struct offset (%u) > remaining (%u)\n",
					i, dt_struct_offset, hdr->kernel_size - i);
			continue;
		}

		// Check that fdt_node_header.tag of first node is FDT_BEGIN_NODE
		uint32_t dt_begin_node = fdt32_to_cpu(*(uint32_t *)(kernel + i + dt_struct_offset));
		if (dt_begin_node != FDT_BEGIN_NODE) {
			fprintf(stderr, "Invalid DTB detection at 0x%x: "
							"header tag of first node != FDT_BEGIN_NODE\n", i);
			continue;
		}

		dtb = kernel + i;
		dt_size = hdr->kernel_size - i;
		hdr->kernel_size = i;
		fprintf(stderr, "DTB             [%u]\n", dt_size);
		break;
	}
}

void boot_img::print_hdr() {
	fprintf(stderr, "HEADER_VER      [%u]\n", header_version());
	fprintf(stderr, "KERNEL_SZ       [%u]\n", hdr->kernel_size);
	fprintf(stderr, "RAMDISK_SZ      [%u]\n", hdr->ramdisk_size);
	fprintf(stderr, "SECOND_SZ       [%u]\n", hdr->second_size);
	fprintf(stderr, "EXTRA_SZ        [%u]\n", extra_size());
	fprintf(stderr, "RECOV_DTBO_SZ   [%u]\n", recovery_dtbo_size());

	uint32_t ver = os_version();
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
		fprintf(stderr, "PATCH_LEVEL     [%d-%02d]\n", y, m);
	}

	fprintf(stderr, "PAGESIZE        [%u]\n", page_size());
	fprintf(stderr, "NAME            [%s]\n", name());
	fprintf(stderr, "CMDLINE         [%.512s%.1024s]\n", cmdline(), extra_cmdline());
	fprintf(stderr, "CHECKSUM        [");
	for (int i = 0; id()[i]; ++i)
		fprintf(stderr, "%02x", id()[i]);
	fprintf(stderr, "]\n");
}

int unpack(const char *image) {
	boot_img boot {};
	int ret = boot.parse_image(image);
	int fd;

	// Dump kernel
	if (COMPRESSED(boot.k_fmt)) {
		fd = creat(KERNEL_FILE, 0644);
		decompress(boot.k_fmt, fd, boot.kernel, boot.hdr->kernel_size);
		close(fd);
	} else {
		dump(boot.kernel, boot.hdr->kernel_size, KERNEL_FILE);
	}

	// Dump dtb
	dump(boot.dtb, boot.dt_size, DTB_FILE);

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
	dump(boot.extra, boot.extra_size(), EXTRA_FILE);

	// Dump recovery_dtbo
	dump(boot.recov_dtbo, boot.recovery_dtbo_size(), RECV_DTBO_FILE);
	return ret;
}

#define file_align() write_zero(fd, align_off(lseek(fd, 0, SEEK_CUR) - header_off, boot.page_size()))
void repack(const char* orig_image, const char* out_image) {
	boot_img boot {};

	off_t header_off, kernel_off, ramdisk_off, second_off, extra_off;

	// Parse original image
	boot.parse_image(orig_image);

	// Reset sizes
	boot.hdr->kernel_size = 0;
	boot.hdr->ramdisk_size = 0;
	boot.hdr->second_size = 0;
	boot.dt_size = 0;

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

	// Skip a page for header
	header_off = lseek(fd, 0, SEEK_CUR);
	write_zero(fd, boot.page_size());

	// kernel
	kernel_off = lseek(fd, 0, SEEK_CUR);
	if (boot.flags & MTK_KERNEL) {
		// Skip MTK header
		write_zero(fd, 512);
	}
	if (access(KERNEL_FILE, R_OK) == 0) {
		if (COMPRESSED(boot.k_fmt)) {
			size_t raw_size;
			void *kernel_raw;
			mmap_ro(KERNEL_FILE, &kernel_raw, &raw_size);
			boot.hdr->kernel_size = compress(boot.k_fmt, fd, kernel_raw, raw_size);
			munmap(kernel_raw, raw_size);
		} else {
			boot.hdr->kernel_size = restore(KERNEL_FILE, fd);
		}
	}

	// dtb
	if (access(DTB_FILE, R_OK) == 0)
		boot.hdr->kernel_size += restore(DTB_FILE, fd);
	file_align();

	// ramdisk
	ramdisk_off = lseek(fd, 0, SEEK_CUR);
	if (boot.flags & MTK_RAMDISK) {
		// Skip MTK header
		write_zero(fd, 512);
	}
	if (access(RAMDISK_FILE, R_OK) == 0) {
		if (COMPRESSED(boot.r_fmt)) {
			size_t cpio_size;
			void *cpio;
			mmap_ro(RAMDISK_FILE, &cpio, &cpio_size);
			boot.hdr->ramdisk_size = compress(boot.r_fmt, fd, cpio, cpio_size);
			munmap(cpio, cpio_size);
		} else {
			boot.hdr->ramdisk_size = restore(RAMDISK_FILE, fd);
		}
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
		boot.extra_size(restore(EXTRA_FILE, fd));
		file_align();
	}

	// recovery_dtbo
	if (access(RECV_DTBO_FILE, R_OK) == 0) {
		boot.recovery_dtbo_offset(lseek(fd, 0, SEEK_CUR));
		boot.recovery_dtbo_size(restore(RECV_DTBO_FILE, fd));
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
	mmap_rw(out_image, (void **) &boot.map_addr, &boot.map_size);

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
	size = boot.extra_size();
	if (size) {
		HASH_update(&ctx, boot.map_addr + extra_off, size);
		HASH_update(&ctx, &size, sizeof(size));
	}
	if (boot.header_version()) {
		size = boot.recovery_dtbo_size();
		HASH_update(&ctx, boot.map_addr + boot.recovery_dtbo_offset(), size);
		HASH_update(&ctx, &size, sizeof(size));
	}
	memset(boot.id(), 0, 32);
	memcpy(boot.id(), HASH_final(&ctx),
		   (boot.flags & SHA256_FLAG) ? SHA256_DIGEST_SIZE : SHA_DIGEST_SIZE);

	// Print new image info
	boot.print_hdr();

	// Try to fix the header
	if (boot.header_version() && boot.header_size() == 0)
		boot.header_size(sizeof(boot_img_hdr));

	// Main header
	memcpy(boot.map_addr + header_off, boot.hdr, boot.hdr_size());

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
