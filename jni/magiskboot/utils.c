#include "magiskboot.h"
#include "elf.h"

void mmap_ro(const char *filename, unsigned char **buf, size_t *size) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		error(1, "Cannot open %s", filename);
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = mmap(NULL, *size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
}

void mmap_rw(const char *filename, unsigned char **buf, size_t *size) {
	int fd = open(filename, O_RDWR);
	if (fd < 0)
		error(1, "Cannot open %s", filename);
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = mmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
}

file_t check_type(const unsigned char *buf) {
	if (memcmp(buf, CHROMEOS_MAGIC, CHROMEOS_MAGIC_SIZE) == 0) {
		return CHROMEOS;
	} else if (memcmp(buf, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0) {
		return AOSP;
	} else if (memcmp(buf, ELF_MAGIC, ELF_MAGIC_SIZE) == 0) {
		return ELF;
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
	} else if ( (  memcmp(buf, "\x04\x22\x4d\x18", 4) == 0 
				|| memcmp(buf, "\x03\x21\x4c\x18", 4) == 0) 
				|| memcmp(buf, "\x02\x21\x4c\x18", 4) == 0) {
		return LZ4;
	} else if (memcmp(buf, "\x88\x16\x88\x58", 4) == 0) {
		return MTK;
	} else if (memcmp(buf, "QCDT", 4) == 0) {
		return QCDT;
	} else {
		return UNKNOWN;
	}
}

void mem_align(size_t *pos, size_t align) {
	size_t mask = align - 1;
	if (*pos & mask) {
		*pos += align - (*pos & mask);
	}
}

void file_align(int fd, size_t align) {
	size_t pos = lseek(fd, 0, SEEK_CUR);
	size_t mask = align - 1;
	if (pos & mask) {
		pos += align - (pos & mask);
		ftruncate(fd, pos);
		lseek(fd, 0, SEEK_END);
	}
}

int open_new(const char *filename) {
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		error(1, "Unable to create %s", filename);
	return fd;
}
