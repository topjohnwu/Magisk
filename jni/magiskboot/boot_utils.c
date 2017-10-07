#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

#include "utils.h"

void mmap_ro(const char *filename, void **buf, size_t *size) {
	int fd = xopen(filename, O_RDONLY);
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = *size > 0 ? xmmap(NULL, *size, PROT_READ, MAP_SHARED, fd, 0) : NULL;
	close(fd);
}

void mmap_rw(const char *filename, void **buf, size_t *size) {
	int fd = xopen(filename, O_RDWR);
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = *size > 0 ? xmmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) : NULL;
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

void *patch_init_rc(char *data, uint32_t *size) {
	int injected = 0;
	char *new_data = xmalloc(*size + 23);
	char *old_data = data;
	uint32_t pos = 0;

	for (char *tok = strsep(&old_data, "\n"); tok; tok = strsep(&old_data, "\n")) {
		if (!injected && strncmp(tok, "import", 6) == 0) {
			if (strstr(tok, "init.magisk.rc")) {
				injected = 1;
			} else {
				fprintf(stderr, "Inject [import /init.magisk.rc] to [init.rc]\n");
				strcpy(new_data + pos, "import /init.magisk.rc\n");
				pos += 23;
				injected = 1;
			}
		} else if (strstr(tok, "selinux.reload_policy")) {
			continue;
		}
		// Copy the line
		strcpy(new_data + pos, tok);
		pos += strlen(tok);
		new_data[pos++] = '\n';
	}

	*size = pos;
	return new_data;
}

int check_verity_pattern(const char *s) {
	int pos = 0;
	if (s[0] == ',') ++pos;
	if (strncmp(s + pos, "verify", 6) != 0) return -1;
	pos += 6;
	if (s[pos] == '=') {
		while (s[pos] != '\0' && s[pos] != ' ' && s[pos] != '\n' && s[pos] != ',') ++pos;
	}
	return pos;
}

int check_encryption_pattern(const char *s) {
	const char *encrypt_list[] = { "forceencrypt", "forcefdeorfbe", NULL };
	for (int i = 0 ; encrypt_list[i]; ++i) {
		int len = strlen(encrypt_list[i]);
		if (strncmp(s, encrypt_list[i], len) == 0)
			return len;
	}
	return -1;
}
