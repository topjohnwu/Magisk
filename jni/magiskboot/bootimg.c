#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "bootimg.h"
#include "magiskboot.h"
#include "utils.h"
#include "logging.h"

static void dump(void *buf, size_t size, const char *filename) {
	int fd = open_new(filename);
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

static void print_hdr(const boot_img_hdr *hdr) {
	fprintf(stderr, "KERNEL [%d] @ 0x%08x\n", hdr->kernel_size, hdr->kernel_addr);
	fprintf(stderr, "RAMDISK [%d] @ 0x%08x\n", hdr->ramdisk_size, hdr->ramdisk_addr);
	fprintf(stderr, "SECOND [%d] @ 0x%08x\n", hdr->second_size, hdr->second_addr);
	fprintf(stderr, "EXTRA [%d] @ 0x%08x\n", hdr->extra_size, hdr->tags_addr);
	fprintf(stderr, "PAGESIZE [%d]\n", hdr->page_size);
	if (hdr->os_version != 0) {
		int a,b,c,y,m = 0;
		int os_version, os_patch_level;
		os_version = hdr->os_version >> 11;
		os_patch_level = hdr->os_version & 0x7ff;

		a = (os_version >> 14) & 0x7f;
		b = (os_version >> 7) & 0x7f;
		c = os_version & 0x7f;
		fprintf(stderr, "OS_VERSION [%d.%d.%d]\n", a, b, c);

		y = (os_patch_level >> 4) + 2000;
		m = os_patch_level & 0xf;
		fprintf(stderr, "PATCH_LEVEL [%d-%02d]\n", y, m);
	}
	fprintf(stderr, "NAME [%s]\n", hdr->name);
	fprintf(stderr, "CMDLINE [%s]\n", hdr->cmdline);
	fprintf(stderr, "\n");
}

int parse_img(void *orig, size_t size, boot_img *boot) {
	void *base, *end;
	size_t pos = 0;
	int ret = 0;
	memset(boot, 0, sizeof(*boot));
	for(base = orig, end = orig + size; base < end; base += 256, size -= 256) {
		switch (check_type(base)) {
		case CHROMEOS:
			// The caller should know it's chromeos, as it needs additional signing
			ret = 2;
			continue;
		case ELF32:
			exit(3);
		case ELF64:
			exit(4);
		case AOSP:
			// Read the header
			memcpy(&boot->hdr, base, sizeof(boot->hdr));
			pos += boot->hdr.page_size;

			print_hdr(&boot->hdr);

			boot->kernel = base + pos;
			pos += boot->hdr.kernel_size;
			mem_align(&pos, boot->hdr.page_size);

			boot->ramdisk = base + pos;
			pos += boot->hdr.ramdisk_size;
			mem_align(&pos, boot->hdr.page_size);

			if (boot->hdr.second_size) {
				boot->second = base + pos;
				pos += boot->hdr.second_size;
				mem_align(&pos, boot->hdr.page_size);
			}

			if (boot->hdr.extra_size) {
				boot->extra = base + pos;
				pos += boot->hdr.extra_size;
				mem_align(&pos, boot->hdr.page_size);
			}

			if (pos < size) {
				boot->tail = base + pos;
				boot->tail_size = end - base - pos;
			}

			// Search for dtb in kernel
			for (int i = 0; i < boot->hdr.kernel_size; ++i) {
				if (memcmp(boot->kernel + i, DTB_MAGIC, 4) == 0) {
					boot->dtb = boot->kernel + i;
					boot->dt_size = boot->hdr.kernel_size - i;
					boot->hdr.kernel_size = i;
					fprintf(stderr, "DTB [%d]\n", boot->dt_size);
				}
			}

			boot->ramdisk_type = check_type(boot->ramdisk);
			boot->kernel_type = check_type(boot->kernel);

			// Check MTK
			if (boot->kernel_type == MTK) {
				fprintf(stderr, "MTK_KERNEL_HDR [512]\n");
				boot->flags |= MTK_KERNEL;
				memcpy(&boot->mtk_kernel_hdr, boot->kernel, sizeof(mtk_hdr));
				boot->kernel += 512;
				boot->hdr.kernel_size -= 512;
				boot->kernel_type = check_type(boot->kernel);
			}
			if (boot->ramdisk_type == MTK) {
				fprintf(stderr, "MTK_RAMDISK_HDR [512]\n");
				boot->flags |= MTK_RAMDISK;
				memcpy(&boot->mtk_ramdisk_hdr, boot->ramdisk, sizeof(mtk_hdr));
				boot->ramdisk += 512;
				boot->hdr.ramdisk_size -= 512;
				boot->ramdisk_type = check_type(boot->ramdisk + 512);
			}

			char fmt[16];

			get_type_name(boot->kernel_type, fmt);
			fprintf(stderr, "KERNEL_FMT [%s]\n", fmt);
			get_type_name(boot->ramdisk_type, fmt);
			fprintf(stderr, "RAMDISK_FMT [%s]\n", fmt);
			fprintf(stderr, "\n");

			return ret;
		default:
			continue;
		}
	}
	LOGE("No boot image magic found!\n");
	return 1;
}

void unpack(const char* image) {
	size_t size;
	void *orig;
	mmap_ro(image, &orig, &size);
	int fd;
	boot_img boot;

	// Parse image
	fprintf(stderr, "Parsing boot image: [%s]\n\n", image);
	int ret = parse_img(orig, size, &boot);

	// Dump kernel
	if (COMPRESSED(boot.kernel_type)) {
		fd = open_new(KERNEL_FILE);
		decomp(boot.kernel_type, fd, boot.kernel, boot.hdr.kernel_size);
		close(fd);
	} else {
		dump(boot.kernel, boot.hdr.kernel_size, KERNEL_FILE);
	}

	if (boot.dt_size) {
		// Dump dtb
		dump(boot.dtb, boot.dt_size, DTB_FILE);
	}

	// Dump ramdisk
	if (COMPRESSED(boot.ramdisk_type)) {
		fd = open_new(RAMDISK_FILE);
		decomp(boot.ramdisk_type, fd, boot.ramdisk, boot.hdr.ramdisk_size);
		close(fd);
	} else {
		dump(boot.ramdisk, boot.hdr.ramdisk_size, RAMDISK_FILE ".raw");
		LOGE("Unknown ramdisk format! Dumped to %s\n", RAMDISK_FILE ".raw");
	}

	if (boot.hdr.second_size) {
		// Dump second
		dump(boot.second, boot.hdr.second_size, SECOND_FILE);
	}

	if (boot.hdr.extra_size) {
		// Dump extra
		dump(boot.extra, boot.hdr.extra_size, EXTRA_FILE);
	}

	munmap(orig, size);
	exit(ret);
}

void repack(const char* orig_image, const char* out_image) {
	size_t size;
	void *orig;
	boot_img boot;

	// There are possible two MTK headers
	size_t mtk_kernel_off, mtk_ramdisk_off;

	// Load original image
	mmap_ro(orig_image, &orig, &size);

	// Parse original image
	fprintf(stderr, "Parsing boot image: [%s]\n\n", orig_image);
	parse_img(orig, size, &boot);

	fprintf(stderr, "Repack to boot image: [%s]\n\n", out_image);

	// Create new image
	int fd = open_new(out_image);

	// Skip a page for header
	write_zero(fd, boot.hdr.page_size);

	if (boot.flags & MTK_KERNEL) {
		// Record position and skip MTK header
		mtk_kernel_off = lseek(fd, 0, SEEK_CUR);
		write_zero(fd, 512);
	}
	if (COMPRESSED(boot.kernel_type)) {
		size_t raw_size;
		void *kernel_raw;
		mmap_ro(KERNEL_FILE, &kernel_raw, &raw_size);
		boot.hdr.kernel_size = comp(boot.kernel_type, fd, kernel_raw, raw_size);
		munmap(kernel_raw, raw_size);
	} else {
		boot.hdr.kernel_size = restore(KERNEL_FILE, fd);
	}
	// Restore dtb
	if (boot.dt_size && access(DTB_FILE, R_OK) == 0) {
		boot.hdr.kernel_size += restore(DTB_FILE, fd);
	}
	file_align(fd, boot.hdr.page_size, 1);

	if (boot.flags & MTK_RAMDISK) {
		// Record position and skip MTK header
		mtk_ramdisk_off = lseek(fd, 0, SEEK_CUR);
		write_zero(fd, 512);
	}
	if (access(RAMDISK_FILE, R_OK) == 0) {
		// If we found raw cpio, compress to original format
		size_t cpio_size;
		void *cpio;
		mmap_ro(RAMDISK_FILE, &cpio, &cpio_size);
		boot.hdr.ramdisk_size = comp(boot.ramdisk_type, fd, cpio, cpio_size);
		munmap(cpio, cpio_size);
	} else {
		// Find compressed ramdisk
		char name[PATH_MAX];
		int found = 0;
		for (int i = 0; SUP_EXT_LIST[i]; ++i) {
			sprintf(name, "%s.%s", RAMDISK_FILE, SUP_EXT_LIST[i]);
			if (access(name, R_OK) == 0) {
				found = 1;
				break;
			}
		}
		if (!found)
			LOGE("No ramdisk exists!\n");
		boot.hdr.ramdisk_size = restore(name, fd);
	}
	file_align(fd, boot.hdr.page_size, 1);

	// Restore second
	if (boot.hdr.second_size && access(SECOND_FILE, R_OK) == 0) {
		boot.hdr.second_size = restore(SECOND_FILE, fd);
		file_align(fd, boot.hdr.page_size, 1);
	}

	// Restore extra
	if (boot.hdr.extra_size && access(EXTRA_FILE, R_OK) == 0) {
		boot.hdr.extra_size = restore(EXTRA_FILE, fd);
		file_align(fd, boot.hdr.page_size, 1);
	}

	// Check tail info, currently only for LG Bump and Samsung SEANDROIDENFORCE
	if (boot.tail_size >= 16) {
		if (memcmp(boot.tail, "SEANDROIDENFORCE", 16) == 0 ||
			memcmp(boot.tail, LG_BUMP_MAGIC, 16) == 0 ) {
			restore_buf(fd, boot.tail, 16);
		}
	}

	// Write MTK headers back
	if (boot.flags & MTK_KERNEL) {
		lseek(fd, mtk_kernel_off, SEEK_SET);
		boot.mtk_kernel_hdr.size = boot.hdr.kernel_size;
		boot.hdr.kernel_size += 512;
		restore_buf(fd, &boot.mtk_kernel_hdr, sizeof(mtk_hdr));
	}
	if (boot.flags & MTK_RAMDISK) {
		lseek(fd, mtk_ramdisk_off, SEEK_SET);
		boot.mtk_ramdisk_hdr.size = boot.hdr.ramdisk_size;
		boot.hdr.ramdisk_size += 512;
		restore_buf(fd, &boot.mtk_ramdisk_hdr, sizeof(mtk_hdr));
	}
	// Main header
	lseek(fd, 0, SEEK_SET);
	restore_buf(fd, &boot.hdr, sizeof(boot.hdr));

	// Print new image info
	print_hdr(&boot.hdr);

	munmap(orig, size);
	close(fd);
}

