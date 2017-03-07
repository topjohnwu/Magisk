#include "magiskboot.h"

static size_t restore(const char *filename, int fd) {
	int ifd = open(filename, O_RDONLY);
	if (ifd < 0)
		error(1, "Cannot open %s\n", filename);

	size_t size = lseek(ifd, 0, SEEK_END);
	lseek(ifd, 0, SEEK_SET);
	if (sendfile(fd, ifd, NULL, size) != size) {
		error(1, "Cannot write %s\n", filename);
	}
	close(ifd);
	return size;
}

static void restore_buf(const void *buf, size_t size, int fd) {
	if (write(fd, buf, size) != size) {
		error(1, "Cannot dump from input file\n");
	}
}

void repack(const char* orig_image, const char* out_image) {
	size_t size;
	unsigned char *orig;
	char name[PATH_MAX];

	// Load original image
	mmap_ro(orig_image, &orig, &size);

	// Parse original image
	printf("\nParsing boot image: [%s]\n\n", orig_image);
	parse_img(orig, size);

	// Create new image
	int fd = open_new(out_image);

	// Set all sizes to 0
	hdr.kernel_size = 0;
	hdr.ramdisk_size = 0;
	hdr.second_size = 0;
	hdr.dt_size = 0;

	// Skip a page for header
	ftruncate(fd, hdr.page_size);
	lseek(fd, 0, SEEK_END);

	// Restore kernel
	if (mtk_kernel) {
		restore_buf(kernel, 512, fd);
		hdr.kernel_size += 512;
	}
	hdr.kernel_size += restore(KERNEL_FILE, fd);
	file_align(fd, hdr.page_size, 1);

	// Restore ramdisk
	if (mtk_ramdisk) {
		restore_buf(ramdisk, 512, fd);
		hdr.ramdisk_size += 512;
	}

	if (access(RAMDISK_FILE, R_OK) == 0) {
		// If we found raw cpio, compress to original format

		// Before we start, clean up previous compressed files
		for (int i = 0; i < SUP_NUM; ++i) {
			sprintf(name, "%s.%s", RAMDISK_FILE, SUP_EXT_LIST[i]);
			unlink(name);
		}

		size_t cpio_size;
		unsigned char *cpio;
		mmap_ro(RAMDISK_FILE, &cpio, &cpio_size);

		if (comp(ramdisk_type, RAMDISK_FILE, cpio, cpio_size))
			error(1, "Unsupported ramdisk format!");

		munmap(cpio, cpio_size);
	}

	int found = 0;
	for (int i = 0; i < SUP_NUM; ++i) {
		sprintf(name, "%s.%s", RAMDISK_FILE, SUP_EXT_LIST[i]);
		if (access(name, R_OK) == 0) {
			ramdisk_type = SUP_TYPE_LIST[i];
			found = 1;
			break;
		}
	}
	if (!found)
		error(1, "No ramdisk exists!");
	hdr.ramdisk_size += restore(name, fd);
	file_align(fd, hdr.page_size, 1);

	// Restore second
	if (access(SECOND_FILE, R_OK) == 0) {
		hdr.second_size += restore(SECOND_FILE, fd);
		file_align(fd, hdr.page_size, 1);
	}

	// Restore dtb
	if (access(DTB_FILE, R_OK) == 0) {
		hdr.dt_size += restore(DTB_FILE, fd);
		file_align(fd, hdr.page_size, 1);
	}

	// Write header back
	printf("\nRepack to boot image: [%s]\n\n", out_image);
	print_info();
	lseek(fd, 0, SEEK_SET);
	write(fd, &hdr, sizeof(hdr));

	munmap(orig, size);
	if (lseek(fd, 0, SEEK_CUR) > size) {
		error(2, "Boot partition too small!");
	}
	close(fd);
}
