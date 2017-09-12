#include "magiskboot.h"

char *SUP_LIST[] = { "gzip", "xz", "lzma", "bzip2", "lz4", "lz4_legacy", NULL };
char *SUP_EXT_LIST[] = { "gz", "xz", "lzma", "bz2", "lz4", "lz4", NULL };
file_t SUP_TYPE_LIST[] = { GZIP, XZ, LZMA, BZIP2, LZ4, LZ4_LEGACY, 0 };

void mmap_ro(const char *filename, void **buf, size_t *size) {
	int fd = xopen(filename, O_RDONLY);
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = xmmap(NULL, *size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
}

void mmap_rw(const char *filename, void **buf, size_t *size) {
	int fd = xopen(filename, O_RDWR);
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = xmmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
}

file_t check_type(const void *buf) {
	if (memcmp(buf, CHROMEOS_MAGIC, 8) == 0) {
		return CHROMEOS;
	} else if (memcmp(buf, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0) {
		return AOSP;
	} else if (memcmp(buf, ELF32_MAGIC, 5) == 0) {
		return ELF32;
	} else if (memcmp(buf, ELF64_MAGIC, 5) == 0) {
		return ELF64;
	} else if (memcmp(buf, GZIP_MAGIC, 4) == 0) {
		return GZIP;
	} else if (memcmp(buf, LZOP_MAGIC, 9) == 0) {
		return LZOP;
	} else if (memcmp(buf, XZ_MAGIC, 6) == 0) {
		return XZ;
	} else if (memcmp(buf, "\x5d\x00\x00", 3) == 0
			&& (((char *)buf)[12] == '\xff' || ((char *)buf)[12] == '\x00')) {
		return LZMA;
	} else if (memcmp(buf, BZIP_MAGIC, 3) == 0) {
		return BZIP2;
	} else if (memcmp(buf, LZ4_MAGIC, 4) == 0) {
		return LZ4;
	} else if (memcmp(buf, LZ4_LEG_MAGIC, 4) == 0) {
		return LZ4_LEGACY;
	} else if (memcmp(buf, MTK_MAGIC, 4) == 0) {
		return MTK;
	} else if (memcmp(buf, DTB_MAGIC, 4) == 0) {
		return DTB;
	} else {
		return UNKNOWN;
	}
}

void get_type_name(file_t type, char *name) {
	char *s;
	switch (type) {
		case CHROMEOS:
			s = "chromeos";
			break;
		case AOSP:
			s = "aosp";
			break;
		case GZIP:
			s = "gzip";
			break;
		case LZOP:
			s = "lzop";
			break;
		case XZ:
			s = "xz";
			break;
		case LZMA:
			s = "lzma";
			break;
		case BZIP2:
			s = "bzip2";
			break;
		case LZ4:
			s = "lz4";
			break;
		case LZ4_LEGACY:
			s = "lz4_legacy";
			break;
		case MTK:
			s = "mtk";
			break;
		case DTB:
			s = "dtb";
			break;
		default:
			s = "raw";
	}
	strcpy(name, s);
}

void write_zero(int fd, size_t size) {
	size_t pos = lseek(fd, 0, SEEK_CUR);
	ftruncate(fd, pos + size);
	lseek(fd, pos + size, SEEK_SET);
}

void mem_align(size_t *pos, size_t align) {
	size_t mask = align - 1;
	if (*pos & mask) {
		*pos += align - (*pos & mask);
	}
}

void file_align(int fd, size_t align, int out) {
	size_t pos = lseek(fd, 0, SEEK_CUR);
	size_t mask = align - 1;
	size_t off;
	if (pos & mask) {
		off = align - (pos & mask);
		if (out) {
			write_zero(fd, off);
		} else {
			lseek(fd, pos + off, SEEK_SET);
		}
	}
}

int open_new(const char *filename) {
	return xopen(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
}

void cleanup() {
	fprintf(stderr, "Cleaning up...\n");
	char name[PATH_MAX];
	unlink(KERNEL_FILE);
	unlink(RAMDISK_FILE);
	unlink(RAMDISK_FILE ".unsupport");
	unlink(SECOND_FILE);
	unlink(DTB_FILE);
	for (int i = 0; SUP_EXT_LIST[i]; ++i) {
		sprintf(name, "%s.%s", RAMDISK_FILE, SUP_EXT_LIST[i]);
		unlink(name);
	}
}
