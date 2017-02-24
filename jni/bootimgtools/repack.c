#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

#include "bootimg.h"

// Global pointer of current positions
static void *ibase, *ipos;
static int ofd, opos;

static size_t dump(const char *filename) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Cannot open %s\n", filename);
		exit(1);
	}
	size_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	if (sendfile(ofd, fd, NULL, size) < 0) {
		fprintf(stderr, "Cannot write %s\n", filename);
		exit(1);
	}
	close(fd);
	opos += size;
	return size;
}

static void dump_buf(size_t size, const void *buf) {
	if (write(ofd, buf, size) < 0) {
		fprintf(stderr, "Cannot dump from input file\n");
		exit(1);
	}
	opos += size;
}

static void in_page_align(uint32_t pagesize) {
	uint32_t itemsize = ipos - ibase, pagemask = pagesize - 1L;
	if (itemsize & pagemask) {
		ipos += pagesize - (itemsize & pagemask);
	}
}

static void out_page_align(uint32_t pagesize) {
	uint32_t pagemask = pagesize - 1L;
	if (opos & pagemask) {
		opos += pagesize - (opos & pagemask);
	}
	ftruncate(ofd, opos);
	lseek(ofd, 0, SEEK_END);
}

static int aosp() {
	printf("AOSP Boot Image Detected\n");

	char *name;
	boot_img_hdr hdr, ihdr;

	// Read the original header
	memcpy(&ihdr, ibase, sizeof(ihdr));
	hdr = ihdr;

	// Set all sizes to 0
	hdr.kernel_size = 0;
	hdr.ramdisk_size = 0;
	hdr.second_size = 0;
	hdr.dt_size = 0;

	// Skip a page
	ftruncate(ofd, hdr.page_size);
	lseek(ofd, 0, SEEK_END);
	opos += hdr.page_size;
	ipos = ibase + hdr.page_size;

	// Dump zImage
	if (memcmp(ipos, "\x88\x16\x88\x58", 4) == 0) {
		printf("Dumping MTK header back to zImage\n");
		dump_buf(512, ipos);
		hdr.kernel_size += 512;
	}
	hdr.kernel_size += dump("kernel");
	ipos += ihdr.kernel_size;
	in_page_align(hdr.page_size);
	out_page_align(hdr.page_size);

	// Dump ramdisk
	if (memcmp(ipos, "\x88\x16\x88\x58", 4) == 0) {
		printf("Dumping MTK header back to ramdisk\n");
		dump_buf(512, ipos);
		hdr.ramdisk_size += 512;
	}
	if (access("ramdisk.gz", R_OK) == 0) {
		name = "ramdisk.gz";
	} else if (access("ramdisk.lzo", R_OK) == 0) {
		name = "ramdisk.lzo";
	} else if (access("ramdisk.xz", R_OK) == 0) {
		name = "ramdisk.xz";
	} else if (access("ramdisk.lzma", R_OK) == 0) {
		name = "ramdisk.lzma";
	} else if (access("ramdisk.bz2", R_OK) == 0) {
		name = "ramdisk.bz2";
	} else if (access("ramdisk.lz4", R_OK) == 0) {
		name = "ramdisk.lz4";
	} else {
		fprintf(stderr, "Ramdisk file doesn't exist!\n");
		return 1;
	}
	hdr.ramdisk_size += dump(name);
	out_page_align(hdr.page_size);

	// Dump second
	if (access("second", R_OK) == 0) {
		hdr.second_size += dump("second");
		out_page_align(hdr.page_size);
	}

	// Dump dtb
	if (access("dtb", R_OK) == 0) {
		hdr.dt_size += dump("dtb");
		out_page_align(hdr.page_size);
	}

	// Write header back
	lseek(ofd, 0, SEEK_SET);
	write(ofd, &hdr, sizeof(hdr));

	return 0;
}

int repack(const char* image) {
	// Load original boot
	int ifd = open(image, O_RDONLY), ret = -1;
	size_t isize = lseek(ifd, 0, SEEK_END);
	lseek(ifd, 0, SEEK_SET);
	void *orig = mmap(NULL, isize, PROT_READ, MAP_SHARED, ifd, 0);

	// Create new boot image
	unlink("new-boot.img");
	ofd = open("new-boot.img", O_RDWR | O_CREAT, 0644);

	// Check headers
	for(ibase = orig; ibase < (orig + isize); ibase += 256) {
		if (memcmp(ibase, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0) {
			ret = aosp();
			break;
		}
	}
	munmap(orig, isize);
	close(ifd);
	return ret;
}
