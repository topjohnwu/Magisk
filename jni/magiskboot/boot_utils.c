#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

#include "utils.h"

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
