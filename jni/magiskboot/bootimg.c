#include "bootimg.h"
#include "magiskboot.h"

static void *kernel, *ramdisk, *second, *dtb, *extra;
static boot_img_hdr hdr;
static int mtk_kernel = 0, mtk_ramdisk = 0;
static file_t ramdisk_type;

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

static void print_info() {
	fprintf(stderr, "KERNEL [%d] @ 0x%08x\n", hdr.kernel_size, hdr.kernel_addr);
	fprintf(stderr, "RAMDISK [%d] @ 0x%08x\n", hdr.ramdisk_size, hdr.ramdisk_addr);
	fprintf(stderr, "SECOND [%d] @ 0x%08x\n", hdr.second_size, hdr.second_addr);
	fprintf(stderr, "DTB [%d] @ 0x%08x\n", hdr.dt_size, hdr.tags_addr);
	fprintf(stderr, "PAGESIZE [%d]\n", hdr.page_size);
	if (hdr.os_version != 0) {
		int a,b,c,y,m = 0;
		int os_version, os_patch_level;
		os_version = hdr.os_version >> 11;
		os_patch_level = hdr.os_version & 0x7ff;

		a = (os_version >> 14) & 0x7f;
		b = (os_version >> 7) & 0x7f;
		c = os_version & 0x7f;
		fprintf(stderr, "OS_VERSION [%d.%d.%d]\n", a, b, c);

		y = (os_patch_level >> 4) + 2000;
		m = os_patch_level & 0xf;
		fprintf(stderr, "PATCH_LEVEL [%d-%02d]\n", y, m);
	}
	fprintf(stderr, "NAME [%s]\n", hdr.name);
	fprintf(stderr, "CMDLINE [%s]\n", hdr.cmdline);

	char cmp[16];

	get_type_name(ramdisk_type, cmp);
	fprintf(stderr, "RAMDISK_COMP [%s]\n", cmp);
	fprintf(stderr, "\n");
}

int parse_img(void *orig, size_t size) {
	void *base, *end;
	size_t pos = 0;
	int ret = 0;
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
			memcpy(&hdr, base, sizeof(hdr));
			pos += hdr.page_size;

			// Kernel position
			kernel = base + pos;
			pos += hdr.kernel_size;
			mem_align(&pos, hdr.page_size);

			// Ramdisk position
			ramdisk = base + pos;
			pos += hdr.ramdisk_size;
			mem_align(&pos, hdr.page_size);

			if (hdr.second_size) {
				// Second position
				second = base + pos;
				pos += hdr.second_size;
				mem_align(&pos, hdr.page_size);
			}

			if (hdr.dt_size) {
				// dtb position
				dtb = base + pos;
				pos += hdr.dt_size;
				mem_align(&pos, hdr.page_size);
			}

			if (pos < size) {
				extra = base + pos;
			}

			// Check ramdisk compression type
			ramdisk_type = check_type(ramdisk);

			// Check MTK
			if (check_type(kernel) == MTK) {
				fprintf(stderr, "MTK header found in kernel\n");
				mtk_kernel = 1;
			}
			if (ramdisk_type == MTK) {
				fprintf(stderr, "MTK header found in ramdisk\n");
				mtk_ramdisk = 1;
				ramdisk_type = check_type(ramdisk + 512);
			}

			// Print info
			print_info();
			return ret;
		default:
			continue;
		}
	}
	LOGE(1, "No boot image magic found!\n");
}

void unpack(const char* image) {
	size_t size;
	void *orig;
	mmap_ro(image, &orig, &size);

	// Parse image
	fprintf(stderr, "Parsing boot image: [%s]\n\n", image);
	int ret = parse_img(orig, size);

	// Dump kernel
	if (mtk_kernel) {
		kernel += 512;
		hdr.kernel_size -= 512;
	}
	dump(kernel, hdr.kernel_size, KERNEL_FILE);

	// Dump ramdisk
	if (mtk_ramdisk) {
		ramdisk += 512;
		hdr.ramdisk_size -= 512;
	}
	if (decomp(ramdisk_type, RAMDISK_FILE, ramdisk, hdr.ramdisk_size)) {
		// Dump the compressed ramdisk
		dump(ramdisk, hdr.ramdisk_size, RAMDISK_FILE ".raw");
		LOGE(1, "Unknown ramdisk format! Dumped to %s\n", RAMDISK_FILE ".raw");
	}

	if (hdr.second_size) {
		// Dump second
		dump(second, hdr.second_size, SECOND_FILE);
	}

	if (hdr.dt_size) {
		// Dump dtb
		dump(dtb, hdr.dt_size, DTB_FILE);
	}

	munmap(orig, size);
	exit(ret);
}

void repack(const char* orig_image, const char* out_image) {
	size_t size;
	void *orig;
	char name[PATH_MAX];

	// There are possible two MTK headers
	mtk_hdr mtk_kernel_hdr, mtk_ramdisk_hdr;
	size_t mtk_kernel_off, mtk_ramdisk_off;

	// Load original image
	mmap_ro(orig_image, &orig, &size);

	// Parse original image
	fprintf(stderr, "Parsing boot image: [%s]\n\n", orig_image);
	parse_img(orig, size);

	fprintf(stderr, "Repack to boot image: [%s]\n\n", out_image);

	// Create new image
	int fd = open_new(out_image);

	// Set all sizes to 0
	hdr.kernel_size = 0;
	hdr.ramdisk_size = 0;
	hdr.second_size = 0;
	hdr.dt_size = 0;

	// Skip a page for header
	write_zero(fd, hdr.page_size);

	// Restore kernel
	if (mtk_kernel) {
		mtk_kernel_off = lseek(fd, 0, SEEK_CUR);
		restore_buf(fd, kernel, 512);
		memcpy(&mtk_kernel_hdr, kernel, sizeof(mtk_kernel_hdr));
	}
	hdr.kernel_size = restore(KERNEL_FILE, fd);
	file_align(fd, hdr.page_size, 1);

	// Restore ramdisk
	if (mtk_ramdisk) {
		mtk_ramdisk_off = lseek(fd, 0, SEEK_CUR);
		restore_buf(fd, ramdisk, 512);
		memcpy(&mtk_ramdisk_hdr, ramdisk, sizeof(mtk_ramdisk_hdr));
	}
	if (access(RAMDISK_FILE, R_OK) == 0) {
		// If we found raw cpio, compress to original format

		// Before we start, clean up previous compressed files
		for (int i = 0; SUP_EXT_LIST[i]; ++i) {
			sprintf(name, "%s.%s", RAMDISK_FILE, SUP_EXT_LIST[i]);
			unlink(name);
		}

		size_t cpio_size;
		void *cpio;
		mmap_ro(RAMDISK_FILE, &cpio, &cpio_size);

		if (comp(ramdisk_type, RAMDISK_FILE, cpio, cpio_size))
			LOGE(1, "Unsupported ramdisk format!\n");

		munmap(cpio, cpio_size);
	}

	int found = 0;
	for (int i = 0; SUP_EXT_LIST[i]; ++i) {
		sprintf(name, "%s.%s", RAMDISK_FILE, SUP_EXT_LIST[i]);
		if (access(name, R_OK) == 0) {
			ramdisk_type = SUP_TYPE_LIST[i];
			found = 1;
			break;
		}
	}
	if (!found)
		LOGE(1, "No ramdisk exists!\n");
	hdr.ramdisk_size = restore(name, fd);
	file_align(fd, hdr.page_size, 1);

	// Restore second
	if (access(SECOND_FILE, R_OK) == 0) {
		hdr.second_size = restore(SECOND_FILE, fd);
		file_align(fd, hdr.page_size, 1);
	}

	// Restore dtb
	if (access(DTB_FILE, R_OK) == 0) {
		hdr.dt_size = restore(DTB_FILE, fd);
		file_align(fd, hdr.page_size, 1);
	}

	// Check extra info, currently only for LG Bump and Samsung SEANDROIDENFORCE
	if (extra) {
		if (memcmp(extra, "SEANDROIDENFORCE", 16) == 0 ||
			memcmp(extra, LG_BUMP_MAGIC, 16) == 0 ) {
			restore_buf(fd, extra, 16);
		}
	}

	// Write headers back
	if (mtk_kernel) {
		lseek(fd, mtk_kernel_off, SEEK_SET);
		mtk_kernel_hdr.size = hdr.kernel_size;
		hdr.kernel_size += 512;
		restore_buf(fd, &mtk_kernel_hdr, sizeof(mtk_kernel_hdr));
	}
	if (mtk_ramdisk) {
		lseek(fd, mtk_ramdisk_off, SEEK_SET);
		mtk_ramdisk_hdr.size = hdr.ramdisk_size;
		hdr.ramdisk_size += 512;
		restore_buf(fd, &mtk_ramdisk_hdr, sizeof(mtk_ramdisk_hdr));
	}
	// Main header
	lseek(fd, 0, SEEK_SET);
	restore_buf(fd, &hdr, sizeof(hdr));

	// Print new image info
	print_info();

	munmap(orig, size);
	close(fd);
}

