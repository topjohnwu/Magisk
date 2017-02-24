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
static unsigned char *base, *pos;

static void dump(size_t size, const char *filename) {
	unlink(filename);
	int ofd = open(filename, O_WRONLY | O_CREAT, 0644);
	assert(ofd >= 0);
	int ret = write(ofd, pos, size);
	assert(ret == size);
	close(ofd);
	pos += size;
}

static void page_align(uint32_t pagesize) {
	uint32_t itemsize = pos - base, pagemask = pagesize - 1L;
	if (itemsize & pagemask) {
		pos += pagesize - (itemsize & pagemask);
	}
}

static int aosp() {
	printf("AOSP Boot Image Detected\n");

	char name[PATH_MAX], *ext;

	// Read the header
	boot_img_hdr hdr;
	memcpy(&hdr, base, sizeof(hdr));

	pos = base + hdr.page_size;

	// Dump zImage
	if (memcmp(pos, "\x88\x16\x88\x58", 4) == 0) {
		printf("MTK header found in zImage\n");
		pos += 512;
		hdr.kernel_size -= 512;
	}
	dump(hdr.kernel_size, "kernel");
	page_align(hdr.page_size);

	// Dump ramdisk
	if (memcmp(pos, "\x88\x16\x88\x58", 4) == 0) {
		printf("MTK header found in ramdisk\n");
		pos += 512;
		hdr.ramdisk_size -= 512;
	}
	// Compression detection
	if (memcmp(pos, "\x1f\x8b\x08\x00", 4) == 0) {
		// gzip header
		printf("gzip ramdisk format detected!\n");
		ext = "gz";
	} else if (memcmp(pos, "\x89\x4c\x5a\x4f\x00\x0d\x0a\x1a\x0a", 9) == 0) {
		// lzop header
		printf("lzop ramdisk format detected!\n");
		ext = "lzo";
	} else if (memcmp(pos, "\xfd""7zXZ\x00", 6) == 0) {
		// xz header
		printf("xz ramdisk format detected!\n");
		ext = "xz";
	} else if (memcmp(pos, "\x5d\x00\x00", 3) == 0 
			&& (pos[12] == (unsigned char) '\xff' || pos[12] == (unsigned char) '\x00')) {
		// lzma header
		printf("lzma ramdisk format detected!\n");
		ext = "lzma";
	} else if (memcmp(pos, "BZh", 3) == 0) {
		// bzip2 header
		printf("bzip2 ramdisk format detected!\n");
		ext = "bz2";
	} else if ( (  memcmp(pos, "\x04\x22\x4d\x18", 4) == 0 
				|| memcmp(pos, "\x03\x21\x4c\x18", 4) == 0) 
				|| memcmp(pos, "\x02\x21\x4c\x18", 4) == 0) {
		// lz4 header
		printf("lz4 ramdisk format detected!\n");
		ext = "lz4";
	} else {
		fprintf(stderr, "Unknown ramdisk format!\n");
		return 1;
	}
	sprintf(name, "%s.%s", "ramdisk", ext);
	dump(hdr.ramdisk_size, name);
	page_align(hdr.page_size);

	if (hdr.second_size) {
		// Dump second
		dump(hdr.second_size, "second");
		page_align(hdr.page_size);
	}

	if (hdr.dt_size) {
		// Dump dtb
		dump(hdr.dt_size, "dtb");
		page_align(hdr.page_size);
	}

	return 0;
}

int unpack(const char* image) {
	int fd = open(image, O_RDONLY), ret = 0;
	size_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	unsigned char *orig = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

	// Check headers
	for(base = orig; base < (orig + size); base += 256) {
		if (memcmp(base, CHROMEOS_MAGIC, CHROMEOS_MAGIC_SIZE) == 0) {
			dump(0, "chromeos");
		} else if (memcmp(base, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0) {
			ret = aosp();
			break;
		}
	}
	munmap(orig, size);
	close(fd);
	return ret;
}

