#include "magiskboot.h"

static void dump(unsigned char *buf, size_t size, const char *filename) {
	int fd = open_new(filename);
	if (write(fd, buf, size) != size)
		error(1, "Cannot dump %s", filename);
	close(fd);
}

void unpack(const char* image) {
	size_t size;
	unsigned char *orig;
	mmap_ro(image, &orig, &size);

	// Parse image
	parse_img(orig, size);

	if (boot_type == CHROMEOS) {
		// The caller should know it's chromeos, as it needs additional signing
		dump(orig, 0, "chromeos");
	}

	char name[PATH_MAX];

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
		printf("Unsupported format! Please decompress manually!\n");
		switch (ramdisk_type) {
			case LZOP:
				sprintf(name, "%s.%s", RAMDISK_FILE, "lzo");
				break;
			default:
				// Never happens
				break;
		}
		// Dump the compressed ramdisk
		dump(ramdisk, hdr.ramdisk_size, name);
	}

	if (hdr.second_size) {
		// Dump second
		dump(second, hdr.second_size, SECOND_FILE);
	}

	if (hdr.dt_size) {
		if (boot_type == ELF && (dtb_type != QCDT && dtb_type != ELF)) {
			printf("Non QC dtb found in ELF kernel, recreate kernel\n");
			gzip(1, KERNEL_FILE, kernel, hdr.kernel_size);
			int kfp = open(KERNEL_FILE, O_WRONLY | O_APPEND);
			write(kfp, dtb, hdr.dt_size);
			close(kfp);
		} else {
			// Dump dtb
			dump(dtb, hdr.dt_size, DTB_FILE);
		}
	}

	munmap(orig, size);
}

