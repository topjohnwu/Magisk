#include "magiskboot.h"

static void dump(unsigned char *buf, size_t size, const char *filename) {
	int ofd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (ofd < 0)
		error(1, "Cannot open %s", filename);
	if (write(ofd, buf, size) != size)
		error(1, "Cannot dump %s", filename);
	close(ofd);
}

void unpack(const char* image) {
	int fd = open(image, O_RDONLY);
	if (fd < 0)
		error(1, "Cannot open %s", image);

	size_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	unsigned char *orig = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

	// Parse image
	parse_img(orig, size);

	if (boot_type == CHROMEOS) {
		// The caller should know it's chromeos, as it needs additional signing
		dump(base, 0, "chromeos");
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

	switch (ramdisk_type) {
		case GZIP:
			sprintf(name, "%s.%s", RAMDISK_FILE, "gz");
			gzip(0, RAMDISK_FILE, ramdisk, hdr.ramdisk_size);
			break;
		case LZOP:
			sprintf(name, "%s.%s", RAMDISK_FILE, "lzo");
			printf("Unsupported format! Please decompress manually!\n");
			// Dump the compressed ramdisk
			dump(ramdisk, hdr.ramdisk_size, name);
			break;
		case XZ:
			sprintf(name, "%s.%s", RAMDISK_FILE, "xz");
			lzma(0, RAMDISK_FILE, ramdisk, hdr.ramdisk_size);
			break;
		case LZMA:
			sprintf(name, "%s.%s", RAMDISK_FILE, "lzma");
			lzma(0, RAMDISK_FILE, ramdisk, hdr.ramdisk_size);
			break;
		case BZIP2:
			sprintf(name, "%s.%s", RAMDISK_FILE, "bz2");
			bzip2(0, RAMDISK_FILE, ramdisk, hdr.ramdisk_size);
			break;
		case LZ4:
			sprintf(name, "%s.%s", RAMDISK_FILE, "lz4");
			lz4(0, RAMDISK_FILE, ramdisk, hdr.ramdisk_size);
			break;
		default:
			// Never happens
			break;
	}

	if (hdr.second_size) {
		// Dump second
		dump(second, hdr.second_size, SECOND_FILE);
	}

	if (hdr.dt_size) {
		if (boot_type == ELF && (dtb_type != QCDT && dtb_type != ELF	)) {
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
	close(fd);
}

