#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct file_ptr_t {
    int fd;
    void *cookie;
    int (*read_fn)(void*, char*, int);
    int (*write_fn)(void*, const char*, int);
    int (*close_fn)(void*);
} file_ptr_t;

static int fp_read_fn(void *p, char *buf, int sz) {
    intptr_t fd = (intptr_t) p;
    return read(fd, buf, sz);
}

static int fp_write_fn(void *p, const char *buf, int sz) {
    intptr_t fd = (intptr_t) p;
    return write(fd, buf, sz);
}

static int fp_close_fn(void *p) {
    intptr_t fd = (intptr_t) p;
    return close(fd);
}

static void set_fp_fd(file_ptr_t *fp, int fd) {
    fp->fd = fd;
    fp->cookie = NULL;
    fp->read_fn = fp_read_fn;
    fp->write_fn = fp_write_fn;
    fp->close_fn = fp_close_fn;
}

static file_ptr_t __stdio_fp[3];

FILE* stdin  = (FILE *) &__stdio_fp[0];
FILE* stdout = (FILE *) &__stdio_fp[1];
FILE* stderr = (FILE *) &__stdio_fp[2];

void __init_stdio(void) {
    set_fp_fd((file_ptr_t *) stdin, 0);
    set_fp_fd((file_ptr_t *) stdout, 1);
    set_fp_fd((file_ptr_t *) stderr, 2);
}

FILE *fdopen(int fd, const char *mode __attribute__((unused))) {
    file_ptr_t *fp = malloc(sizeof(file_ptr_t));
    set_fp_fd(fp, fd);
    return (FILE *) fp;
}

FILE *funopen(const void* cookie,
              int (*read_fn)(void*, char*, int),
              int (*write_fn)(void*, const char*, int),
              fpos_t (*seek_fn)(void*, fpos_t, int),
              int (*close_fn)(void*)) {
    file_ptr_t *fp = malloc(sizeof(file_ptr_t));
    fp->fd = -1;
    fp->cookie = (void *) cookie;
    fp->read_fn = read_fn;
    fp->write_fn = write_fn;
    fp->close_fn = close_fn;
    return (FILE *) fp;
}

#define fn_arg (fp->fd < 0 ? fp->cookie : ((void*)(intptr_t) fp->fd))

int fclose(FILE *stream) {
    file_ptr_t *fp = (file_ptr_t *) stream;
    int ret = fp->close_fn(fn_arg);
    free(fp);
    return ret;
}

int fileno(FILE *stream) {
    file_ptr_t *fp = (file_ptr_t *) stream;
    return fp->fd;
}

int fputc(int ch, FILE *stream) {
    char c = ch;
    file_ptr_t *fp = (file_ptr_t *) stream;
    return fp->write_fn(fn_arg, &c, 1) >= 0 ? 0 : EOF;
}

size_t fwrite(const void* buf, size_t size, size_t count, FILE* stream) {
    file_ptr_t *fp = (file_ptr_t *) stream;
    int len = size * count;
    int ret = fp->write_fn(fn_arg, buf, len);
    return ret == len ? count : 0;
}

int fputs(const char* s, FILE* stream) {
    file_ptr_t *fp = (file_ptr_t *) stream;
    size_t length = strlen(s);
    return fp->write_fn(fn_arg, s, length) == length ? 0 : EOF;
}

int fgetc(FILE *stream) {
    char ch;
    file_ptr_t *fp = (file_ptr_t *) stream;
    if (fp->read_fn(fn_arg, &ch, 1) == 1) {
        return ch;
    }
    return -1;
}

size_t fread(void *buf, size_t size, size_t count, FILE* stream) {
    file_ptr_t *fp = (file_ptr_t *) stream;
    int len = size * count;
    int ret = fp->read_fn(fn_arg, buf, len);
    return ret == len ? count : 0;
}

void setbuf(FILE* fp, char* buf) {}

#include "tinystdio/tinystdio.c"

struct file_putp {
    FILE *fp;
    int len;
};

static void file_putc(void *data, char ch) {
    struct file_putp *putp = data;
    int r = write(fileno(putp->fp), &ch, 1);
    if (r >= 0)
        putp->len += r;
}

int vfprintf(FILE *stream, const char *format, va_list arg) {
    struct file_putp data;
    data.fp = stream;
    data.len = 0;
    tfp_format(&data, &file_putc, format, arg);
    return data.len;
}

int vasprintf(char **strp, const char *fmt, va_list ap) {
    int size = vsnprintf(NULL, 0, fmt, ap);
    if (size >= 0) {
        *strp = malloc(size + 1);
        vsnprintf(*strp, size, fmt, ap);
    }
    return size;
}

int vprintf(const char *fmt, va_list args) {
    return vfprintf(stdout, fmt, args);
}

int fprintf(FILE *stream, const char *fmt, ...) {
    va_list args;
    int ret;

    va_start(args, fmt);
    ret = vfprintf(stream, fmt, args);
    va_end(args);
    return ret;
}

int printf(const char *fmt, ...) {
    va_list args;
    int ret;

    va_start(args, fmt);
    ret = vfprintf(stdout, fmt, args);
    va_end(args);
    return ret;
}

int sscanf(const char *str, const char *format, ...) {
    va_list ap;
    int retval;

    va_start(ap, format);
    retval = tfp_vsscanf(str, format, ap);
    va_end(ap);
    return retval;
}

// Original source: https://github.com/freebsd/freebsd/blob/master/contrib/file/src/getline.c
// License: BSD, full copyright notice please check original source
ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp) {
    char *ptr, *eptr;

    if (*buf == NULL || *bufsiz == 0) {
        *bufsiz = BUFSIZ;
        if ((*buf = (char *) malloc(*bufsiz)) == NULL)
            return -1;
    }

    for (ptr = *buf, eptr = *buf + *bufsiz;;) {
        int c = fgetc(fp);
        if (c == -1) {
            return ptr == *buf ? -1 : ptr - *buf;
        }
        *ptr++ = c;
        if (c == delimiter) {
            *ptr = '\0';
            return ptr - *buf;
        }
        if (ptr + 2 >= eptr) {
            char *nbuf;
            size_t nbufsiz = *bufsiz * 2;
            ssize_t d = ptr - *buf;
            if ((nbuf = (char *) realloc(*buf, nbufsiz)) == NULL)
                return -1;
            *buf = nbuf;
            *bufsiz = nbufsiz;
            eptr = nbuf + nbufsiz;
            ptr = nbuf + d;
        }
    }
}

ssize_t getline(char **buf, size_t *bufsiz, FILE *fp) {
    return getdelim(buf, bufsiz, '\n', fp);
}
