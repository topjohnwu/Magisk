/* SPDX-License-Identifier: LGPL-2.1 OR MIT */
/*
 * minimal stdio function definitions for NOLIBC
 * Copyright (C) 2017-2021 Willy Tarreau <w@1wt.eu>
 */

#ifndef _NOLIBC_STDIO_H
#define _NOLIBC_STDIO_H

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#ifndef EOF
#define EOF (-1)
#endif

/* Buffering mode used by setvbuf.  */
#define _IOFBF 0	/* Fully buffered. */
#define _IOLBF 1	/* Line buffered. */
#define _IONBF 2	/* No buffering. */

/* just define FILE as a non-empty type. The value of the pointer gives
 * the FD: FILE=~fd for fd>=0 or NULL for fd<0. This way positive FILE
 * are immediately identified as abnormal entries (i.e. possible copies
 * of valid pointers to something else).
 */
typedef struct __sFILE {
	char dummy[1];
} FILE;

extern int vfprintf(FILE *stream, const char *fmt, va_list args);

FILE* stdin  = (FILE*)(intptr_t)~STDIN_FILENO;
FILE* stdout = (FILE*)(intptr_t)~STDOUT_FILENO;
FILE* stderr = (FILE*)(intptr_t)~STDERR_FILENO;

/* provides a FILE* equivalent of fd. The mode is ignored. */
FILE *fdopen(int fd, const char *mode __attribute__((unused)))
{
	if (fd < 0) {
		errno = EBADF;
		return NULL;
	}
	return (FILE*)(intptr_t)~fd;
}

/* provides the fd of stream. */
int fileno(FILE *stream)
{
	intptr_t i = (intptr_t)stream;

	if (i >= 0) {
		errno = EBADF;
		return -1;
	}
	return ~i;
}

/* flush a stream. */
int fflush(FILE *stream)
{
	intptr_t i = (intptr_t)stream;

	/* NULL is valid here. */
	if (i > 0) {
		errno = EBADF;
		return -1;
	}

	/* Don't do anything, nolibc does not support buffering. */
	return 0;
}

/* flush a stream. */
int fclose(FILE *stream)
{
	intptr_t i = (intptr_t)stream;

	if (i >= 0) {
		errno = EBADF;
		return -1;
	}

	if (close(~i))
		return EOF;

	return 0;
}

/* getc(), fgetc(), getchar() */

#define getc(stream) fgetc(stream)

int fgetc(FILE* stream)
{
	unsigned char ch;

	if (read(fileno(stream), &ch, 1) <= 0)
		return EOF;
	return ch;
}

int getchar(void)
{
	return fgetc(stdin);
}


/* putc(), fputc(), putchar() */

#define putc(c, stream) fputc(c, stream)

int fputc(int c, FILE* stream)
{
	unsigned char ch = c;

	if (write(fileno(stream), &ch, 1) <= 0)
		return EOF;
	return ch;
}

int putchar(int c)
{
	return fputc(c, stdout);
}


/* fwrite(), puts(), fputs(). Note that puts() emits '\n' but not fputs(). */

/* internal fwrite()-like function which only takes a size and returns 0 on
 * success or EOF on error. It automatically retries on short writes.
 */
static int _fwrite(const void *buf, size_t size, FILE *stream)
{
	ssize_t ret;
	int fd = fileno(stream);

	while (size) {
		ret = write(fd, buf, size);
		if (ret <= 0)
			return EOF;
		size -= ret;
		buf += ret;
	}
	return 0;
}

size_t fwrite(const void *s, size_t size, size_t nmemb, FILE *stream)
{
	size_t written;

	for (written = 0; written < nmemb; written++) {
		if (_fwrite(s, size, stream) != 0)
			break;
		s += size;
	}
	return written;
}

int fputs(const char *s, FILE *stream)
{
	return _fwrite(s, strlen(s), stream);
}

int puts(const char *s)
{
	if (fputs(s, stdout) == EOF)
		return EOF;
	return putchar('\n');
}


/* fgets() */
char *fgets(char *s, int size, FILE *stream)
{
	int ofs;
	int c;

	for (ofs = 0; ofs + 1 < size;) {
		c = fgetc(stream);
		if (c == EOF)
			break;
		s[ofs++] = c;
		if (c == '\n')
			break;
	}
	if (ofs < size)
		s[ofs] = 0;
	return ofs ? s : NULL;
}

int vprintf(const char *fmt, va_list args)
{
	return vfprintf(stdout, fmt, args);
}

__attribute__((format(printf, 2, 3)))
int fprintf(FILE *stream, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vfprintf(stream, fmt, args);
	va_end(args);
	return ret;
}

__attribute__((format(printf, 1, 2)))
int printf(const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vfprintf(stdout, fmt, args);
	va_end(args);
	return ret;
}

void perror(const char *msg)
{
	fprintf(stderr, "%s%serrno=%d\n", (msg && *msg) ? msg : "", (msg && *msg) ? ": " : "", errno);
}

int setvbuf(FILE *stream __attribute__((unused)),
	    char *buf __attribute__((unused)),
	    int mode,
	    size_t size __attribute__((unused)))
{
	/*
	 * nolibc does not support buffering so this is a nop. Just check mode
	 * is valid as required by the spec.
	 */
	switch (mode) {
	case _IOFBF:
	case _IOLBF:
	case _IONBF:
		break;
	default:
		return EOF;
	}

	return 0;
}

#endif /* _NOLIBC_STDIO_H */
