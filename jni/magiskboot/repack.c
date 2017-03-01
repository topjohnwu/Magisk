#include "magiskboot.h"

// Global pointer of output
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

void repack(const char* image) {
	// Load original image
	int ifd = open(image, O_RDONLY);
	if (ifd < 0)
		error(1, "Cannot open %s", image);
	size_t isize = lseek(ifd, 0, SEEK_END);
	lseek(ifd, 0, SEEK_SET);
	unsigned char *orig = mmap(NULL, isize, PROT_READ, MAP_SHARED, ifd, 0);

	// Parse original image
	parse_img(orig, isize);

	// Create new boot image
	ofd = open("new-boot.img", O_RDWR | O_CREAT | O_TRUNC, 0644);

	char name[PATH_MAX];
	#define EXT_NUM 6
	char *ext_list[EXT_NUM] = { "gz", "lzo", "xz", "lzma", "bz2", "lz4" };
	
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
	if (mtk_kernel) {
		restore_buf(512, kernel);
		hdr.kernel_size += 512;
	}
	hdr.kernel_size += restore(KERNEL_FILE);
	page_align();

	// Restore ramdisk
	if (mtk_ramdisk) {
		restore_buf(512, ramdisk);
		hdr.ramdisk_size += 512;
	}

	if (access(RAMDISK_FILE, R_OK) == 0) {
		// If we found raw cpio, recompress to original format
		int rfd = open(RAMDISK_FILE, O_RDONLY);
		if (rfd < 0)
			error(1, "Cannot open " RAMDISK_FILE);

		size_t cpio_size = lseek(rfd, 0, SEEK_END);
		lseek(rfd, 0, SEEK_SET);
		unsigned char *cpio = mmap(NULL, cpio_size, PROT_READ, MAP_SHARED, rfd, 0);

		switch (ramdisk_type) {
			case GZIP:
				sprintf(name, "%s.%s", RAMDISK_FILE, "gz");
				gzip(1, name, cpio, cpio_size);
				break;
			case LZOP:
				sprintf(name, "%s.%s", RAMDISK_FILE, "lzo");
				error(1, "Unsupported format! Please compress manually!");
				break;
			case XZ:
				sprintf(name, "%s.%s", RAMDISK_FILE, "xz");
				lzma(1, name, cpio, cpio_size);
				break;
			case LZMA:
				sprintf(name, "%s.%s", RAMDISK_FILE, "lzma");
				lzma(2, name, cpio, cpio_size);
				break;
			case BZIP2:
				sprintf(name, "%s.%s", RAMDISK_FILE, "bz2");
				bzip2(1, name, cpio, cpio_size);
				break;
			case LZ4:
				sprintf(name, "%s.%s", RAMDISK_FILE, "lz4");
				lz4(1, name, cpio, cpio_size);
				break;
			default:
				// Never happens
				break;
		}

		munmap(cpio, cpio_size);
		close(rfd);
	} else {
		// If no raw cpio found, find compressed ones
		int found = 0;
		for (int i = 0; i < EXT_NUM; ++i) {
			sprintf(name, "%s.%s", RAMDISK_FILE, ext_list[i]);
			if (access(name, R_OK) == 0) {
				found = 1;
				break;
			}
		}
		if (!found) {
			error(1, "No ramdisk exists!");
		}
	}

	hdr.ramdisk_size += restore(name);
	page_align();

	// Restore second
	if (access(SECOND_FILE, R_OK) == 0) {
		hdr.second_size += restore(SECOND_FILE);
		page_align();
	}

	// Restore dtb
	if (access(DTB_FILE, R_OK) == 0) {
		hdr.dt_size += restore(DTB_FILE);
		page_align();
	}

	// Write header back
	lseek(ofd, 0, SEEK_SET);
	write(ofd, &hdr, sizeof(hdr));

	munmap(orig, isize);
	close(ifd);
	close(ofd);
	if (opos > isize) {
		error(2, "Boot partition too small!");
	}
}
