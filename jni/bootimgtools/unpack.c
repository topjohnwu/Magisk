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

static void dump(unsigned char *buf, size_t size, const char *filename) {
	unlink(filename);
	int ofd = open(filename, O_WRONLY | O_CREAT, 0644);
	if (ofd < 0)
		error(1, "Cannot open %s", filename);
	if (write(ofd, buf, size) != size)
		error(1, "Cannot dump %s", filename);
	close(ofd);
}

int unpack(const char* image) {
	int fd = open(image, O_RDONLY), ret = 0;
	if (fd < 0)
		error(1, "Cannot open %s", image);

	size_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	unsigned char *orig = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

	// Parse images
	for(base = orig; base < (orig + size); base += 256) {
		if (memcmp(base, CHROMEOS_MAGIC, CHROMEOS_MAGIC_SIZE) == 0) {
			dump(base, 0, "chromeos");
		} else if (memcmp(base, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0) {
			parse_aosp();
			break;
		} else if (memcmp(base, ELF_MAGIC, ELF_MAGIC_SIZE) == 0) {
			parse_elf();
			break;
		}
	}

	char name[PATH_MAX], *ext;

	// Dump kernel
	if (memcmp(kernel, "\x88\x16\x88\x58", 4) == 0) {
		printf("MTK header found in kernel\n");
		kernel += 512;
		hdr.kernel_size -= 512;
	}
	dump(kernel, hdr.kernel_size, "kernel");

	// Dump ramdisk
	if (memcmp(ramdisk, "\x88\x16\x88\x58", 4) == 0) {
		printf("MTK header found in ramdisk\n");
		ramdisk += 512;
		hdr.ramdisk_size -= 512;
	}
	// Compression detection
	if (memcmp(ramdisk, "\x1f\x8b\x08\x00", 4) == 0) {
		// gzip header
		printf("COMPRESSION [gzip]\n");
		ext = "gz";
	} else if (memcmp(ramdisk, "\x89\x4c\x5a\x4f\x00\x0d\x0a\x1a\x0a", 9) == 0) {
		// lzop header
		printf("COMPRESSION [lzop]\n");
		ext = "lzo";
	} else if (memcmp(ramdisk, "\xfd""7zXZ\x00", 6) == 0) {
		// xz header
		printf("COMPRESSION [xz]\n");
		ext = "xz";
	} else if (memcmp(ramdisk, "\x5d\x00\x00", 3) == 0 
			&& (ramdisk[12] == (unsigned char) '\xff' || ramdisk[12] == (unsigned char) '\x00')) {
		// lzma header
		printf("COMPRESSION [lzma]\n");
		ext = "lzma";
	} else if (memcmp(ramdisk, "BZh", 3) == 0) {
		// bzip2 header
		printf("COMPRESSION [bzip2]\n");
		ext = "bz2";
	} else if ( (  memcmp(ramdisk, "\x04\x22\x4d\x18", 4) == 0 
				|| memcmp(ramdisk, "\x03\x21\x4c\x18", 4) == 0) 
				|| memcmp(ramdisk, "\x02\x21\x4c\x18", 4) == 0) {
		// lz4 header
		printf("COMPRESSION [lz4]\n");
		ext = "lz4";
	} else {
		error(1, "Unknown ramdisk format!");
	}
	sprintf(name, "%s.%s", "ramdisk", ext);
	dump(ramdisk, hdr.ramdisk_size, name);

	if (hdr.second_size) {
		// Dump second
		dump(second, hdr.second_size, "second");
	}

	if (hdr.dt_size) {
		// Dump dtb
		dump(dtb, hdr.dt_size, "dtb");
	}

	munmap(orig, size);
	close(fd);
	return ret;
}

