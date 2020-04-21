/*
 * Host all missing/incomplete implementation in bionic
 * Copied from various sources
 * */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mntent.h>

#include "missing.hpp"

/* Original source: https://opensource.apple.com/source/cvs/cvs-19/cvs/lib/getline.c
 * License: GPL 2 or later
 * Adjusted to match POSIX */
#define MIN_CHUNK 64
ssize_t __getdelim(char **lineptr, size_t *n, int delim, FILE *stream) {
	size_t nchars_avail;
	char *read_pos;

	if (!lineptr || !n || !stream) {
		errno = EINVAL;
		return -1;
	}

	if (!*lineptr) {
		*n = MIN_CHUNK;
		*lineptr = (char *) malloc(*n);
		if (!*lineptr) {
			errno = ENOMEM;
			return -1;
		}
	}

	nchars_avail = *n;
	read_pos = *lineptr;

	for (;;) {
		int save_errno;
		int c = getc(stream);

		save_errno = errno;

		if (nchars_avail < 2) {
			if (*n > MIN_CHUNK)
				*n *= 2;
			else
				*n += MIN_CHUNK;

			nchars_avail = *n + *lineptr - read_pos;
			*lineptr = (char *) realloc(*lineptr, *n);
			if (!*lineptr) {
				errno = ENOMEM;
				return -1;
			}
			read_pos = *n - nchars_avail + *lineptr;
		}

		if (ferror(stream)) {
			errno = save_errno;
			return -1;
		}

		if (c == EOF) {
			if (read_pos == *lineptr)
				return -1;
			else
				break;
		}

		*read_pos++ = c;
		nchars_avail--;

		if (c == delim)
			break;
	}

	*read_pos = '\0';

	return read_pos - *lineptr;
}

ssize_t __getline(char **lineptr, size_t *n, FILE *stream) {
	return getdelim(lineptr, n, '\n', stream);
}

/* mntent functions are copied from AOSP libc/bionic/mntent.cpp */

struct mntent *__getmntent_r(FILE* fp, struct mntent* e, char* buf, int buf_len) {
	memset(e, 0, sizeof(*e));
	while (fgets(buf, buf_len, fp) != nullptr) {
		// Entries look like "proc /proc proc rw,nosuid,nodev,noexec,relatime 0 0".
		// That is: mnt_fsname mnt_dir mnt_type mnt_opts 0 0.
		int fsname0, fsname1, dir0, dir1, type0, type1, opts0, opts1;
		if (sscanf(buf, " %n%*s%n %n%*s%n %n%*s%n %n%*s%n %d %d",
				   &fsname0, &fsname1, &dir0, &dir1, &type0, &type1, &opts0, &opts1,
				   &e->mnt_freq, &e->mnt_passno) == 2) {
			e->mnt_fsname = &buf[fsname0];
			buf[fsname1] = '\0';
			e->mnt_dir = &buf[dir0];
			buf[dir1] = '\0';
			e->mnt_type = &buf[type0];
			buf[type1] = '\0';
			e->mnt_opts = &buf[opts0];
			buf[opts1] = '\0';
			return e;
		}
	}
	return nullptr;
}

FILE *__setmntent(const char* path, const char* mode) {
	return fopen(path, mode);
}

int __endmntent(FILE* fp) {
	if (fp != nullptr) {
		fclose(fp);
	}
	return 1;
}

char *__hasmntopt(const struct mntent* mnt, const char* opt) {
	char* token = mnt->mnt_opts;
	char* const end = mnt->mnt_opts + strlen(mnt->mnt_opts);
	const size_t optLen = strlen(opt);
	while (token) {
		char* const tokenEnd = token + optLen;
		if (tokenEnd > end) break;
		if (memcmp(token, opt, optLen) == 0 &&
			(*tokenEnd == '\0' || *tokenEnd == ',' || *tokenEnd == '=')) {
			return token;
		}
		token = strchr(token, ',');
		if (token) token++;
	}
	return nullptr;
}

