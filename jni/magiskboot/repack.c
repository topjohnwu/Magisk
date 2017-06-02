#include "magiskboot.h"

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

void repack(const char* orig_image, const char* out_image) {
	size_t size;
	unsigned char *orig;
	char name[PATH_MAX];

	// There are possible two MTK headers
	mtk_hdr mtk_kernel_hdr, mtk_ramdisk_hdr;
	size_t mtk_kernel_off, mtk_ramdisk_off;

	// Load original image
	mmap_ro(orig_image, &orig, &size);

	// Parse original image
	printf("Parsing boot image: [%s]\n\n", orig_image);
	parse_img(orig, size);

	printf("Repack to boot image: [%s]\n\n", out_image);

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
		unsigned char *cpio;
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
			memcmp(extra, "\x41\xa9\xe4\x67\x74\x4d\x1d\x1b\xa4\x29\xf2\xec\xea\x65\x52\x79", 16) == 0 ) {
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
