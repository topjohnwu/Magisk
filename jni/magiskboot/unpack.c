#include "magiskboot.h"

static void dump(unsigned char *buf, size_t size, const char *filename) {
	int fd = open_new(filename);
	xwrite(fd, buf, size);
	close(fd);
}

void unpack(const char* image) {
	size_t size;
	unsigned char *orig;
	mmap_ro(image, &orig, &size);

	// Parse image
	printf("Parsing boot image: [%s]\n\n", image);
	parse_img(orig, size);

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
		dump(ramdisk, hdr.ramdisk_size, RAMDISK_FILE ".unsupport");
		LOGE(1, "Unsupported ramdisk format! Dumped to %s\n", RAMDISK_FILE ".unsupport");
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
}

