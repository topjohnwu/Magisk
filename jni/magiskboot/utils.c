#include "magiskboot.h"

char *SUP_LIST[] = { "gzip", "xz", "lzma", "bzip2", "lz4", "lz4_legacy", NULL };
char *SUP_EXT_LIST[] = { "gz", "xz", "lzma", "bz2", "lz4", "lz4", NULL };
file_t SUP_TYPE_LIST[] = { GZIP, XZ, LZMA, BZIP2, LZ4, LZ4_LEGACY, 0 };

void mmap_ro(const char *filename, unsigned char **buf, size_t *size) {
	int fd = xopen(filename, O_RDONLY);
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = xmmap(NULL, *size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
}

void mmap_rw(const char *filename, unsigned char **buf, size_t *size) {
	int fd = xopen(filename, O_RDWR);
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = xmmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
}

file_t check_type(const unsigned char *buf) {
	if (memcmp(buf, CHROMEOS_MAGIC, 8) == 0) {
		return CHROMEOS;
	} else if (memcmp(buf, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0) {
		return AOSP;
	} else if (memcmp(buf, ELF32_MAGIC, 5) == 0) {
		return ELF32;
	} else if (memcmp(buf, ELF64_MAGIC, 5) == 0) {
		return ELF64;
	} else if (memcmp(buf, "\x1f\x8b\x08\x00", 4) == 0) {
		return GZIP;
	} else if (memcmp(buf, "\x89\x4c\x5a\x4f\x00\x0d\x0a\x1a\x0a", 9) == 0) {
		return LZOP;
	} else if (memcmp(buf, "\xfd""7zXZ\x00", 6) == 0) {
		return XZ;
	} else if (memcmp(buf, "\x5d\x00\x00", 3) == 0 
			&& (buf[12] == (unsigned char) '\xff' || buf[12] == (unsigned char) '\x00')) {
		return LZMA;
	} else if (memcmp(buf, "BZh", 3) == 0) {
		return BZIP2;
	} else if (memcmp(buf, "\x04\x22\x4d\x18", 4) == 0) {
		return LZ4;
	} else if (memcmp(buf, "\x02\x21\x4c\x18", 4) == 0) {
		return LZ4_LEGACY;
	} else if (memcmp(buf, "\x88\x16\x88\x58", 4) == 0) {
		return MTK;
	} else {
		return UNKNOWN;
	}
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

void print_info() {
	printf("KERNEL [%d] @ 0x%08x\n", hdr.kernel_size, hdr.kernel_addr);
	printf("RAMDISK [%d] @ 0x%08x\n", hdr.ramdisk_size, hdr.ramdisk_addr);
	printf("SECOND [%d] @ 0x%08x\n", hdr.second_size, hdr.second_addr);
	printf("DTB [%d] @ 0x%08x\n", hdr.dt_size, hdr.tags_addr);
	printf("PAGESIZE [%d]\n", hdr.page_size);
	if (hdr.os_version != 0) {
		int a,b,c,y,m = 0;
		int os_version, os_patch_level;
		os_version = hdr.os_version >> 11;
		os_patch_level = hdr.os_version & 0x7ff;
		
		a = (os_version >> 14) & 0x7f;
		b = (os_version >> 7) & 0x7f;
		c = os_version & 0x7f;
		printf("OS_VERSION [%d.%d.%d]\n", a, b, c);
		
		y = (os_patch_level >> 4) + 2000;
		m = os_patch_level & 0xf;
		printf("PATCH_LEVEL [%d-%02d]\n", y, m);
	}
	printf("NAME [%s]\n", hdr.name);
	printf("CMDLINE [%s]\n", hdr.cmdline);

	switch (ramdisk_type) {
		case GZIP:
			printf("COMPRESSION [%s]\n", "gzip");
			break;
		case XZ:
			printf("COMPRESSION [%s]\n", "xz");
			break;
		case LZMA:
			printf("COMPRESSION [%s]\n", "lzma");
			break;
		case BZIP2:
			printf("COMPRESSION [%s]\n", "bzip2");
			break;
		case LZ4:
			printf("COMPRESSION [%s]\n", "lz4");
			break;
		case LZ4_LEGACY:
			printf("COMPRESSION [%s]\n", "lz4_legacy");
			break;
		default:
			fprintf(stderr, "Unknown ramdisk format!\n");
	}
	printf("\n");
}

void cleanup() {
	printf("Cleaning up...\n");
	char name[PATH_MAX];
	unlink(KERNEL_FILE);
	unlink(RAMDISK_FILE);
	unlink(RAMDISK_FILE ".unsupport");
	unlink(SECOND_FILE);
	unlink(DTB_FILE);
	unlink(NEW_BOOT);
	for (int i = 0; SUP_EXT_LIST[i]; ++i) {
		sprintf(name, "%s.%s", RAMDISK_FILE, SUP_EXT_LIST[i]);
		unlink(name);
	}
}
