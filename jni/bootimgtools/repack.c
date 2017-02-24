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
#include "elf.h"

// Global pointer of current positions
static int ofd, opos;

static size_t restore(const char *filename) {
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

static void restore_buf(size_t size, const void *buf) {
	if (write(ofd, buf, size) != size) {
		fprintf(stderr, "Cannot dump from input file\n");
		exit(1);
	}
	opos += size;
}

static void page_align() {
	uint32_t pagemask = hdr.page_size - 1L;
	if (opos & pagemask) {
		opos += hdr.page_size - (opos & pagemask);
	}
	ftruncate(ofd, opos);
	lseek(ofd, 0, SEEK_END);
}

int repack(const char* image) {
	// Load original boot
	int ifd = open(image, O_RDONLY), ret = -1;
	if (ifd < 0)
		error(1, "Cannot open %s", image);

	size_t isize = lseek(ifd, 0, SEEK_END);
	lseek(ifd, 0, SEEK_SET);
	unsigned char *orig = mmap(NULL, isize, PROT_READ, MAP_SHARED, ifd, 0);

	// Create new boot image
	unlink("new-boot.img");
	ofd = open("new-boot.img", O_RDWR | O_CREAT, 0644);

	// Parse images
	for(base = orig; base < (orig + isize); base += 256) {
		if (memcmp(base, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0) {
			parse_aosp();
			break;
		} else if (memcmp(base, ELF_MAGIC, ELF_MAGIC_SIZE) == 0) {
			parse_elf();
			break;
		}
	}

	printf("\n");

	char *name;
	
	// Set all sizes to 0
	hdr.kernel_size = 0;
	hdr.ramdisk_size = 0;
	hdr.second_size = 0;
	hdr.dt_size = 0;

	// Skip a page for header
	ftruncate(ofd, hdr.page_size);
	lseek(ofd, 0, SEEK_END);
	opos += hdr.page_size;

	// Restore kernel
	if (memcmp(kernel, "\x88\x16\x88\x58", 4) == 0) {
		printf("Dumping MTK header back to kernel\n");
		restore_buf(512, kernel);
		hdr.kernel_size += 512;
	}
	hdr.kernel_size += restore("kernel");
	page_align();

	// Dump ramdisk
	if (memcmp(ramdisk, "\x88\x16\x88\x58", 4) == 0) {
		printf("Dumping MTK header back to ramdisk\n");
		restore_buf(512, ramdisk);
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
		error(1, "Ramdisk file doesn't exist!");
	}
	hdr.ramdisk_size += restore(name);
	page_align();

	// Dump second
	if (access("second", R_OK) == 0) {
		hdr.second_size += restore("second");
		page_align();
	}

	// Dump dtb
	if (access("dtb", R_OK) == 0) {
		hdr.dt_size += restore("dtb");
		page_align();
	}

	print_header();

	// Write header back
	lseek(ofd, 0, SEEK_SET);
	write(ofd, &hdr, sizeof(hdr));

	munmap(orig, isize);
	close(ifd);
	close(ofd);
	return ret;
}
