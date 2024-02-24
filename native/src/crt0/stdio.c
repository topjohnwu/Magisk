#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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

int sscanf(const char *str, const char *format, ...) {
    va_list ap;
    int retval;

    va_start(ap, format);
    retval = tfp_vsscanf(str, format, ap);
    va_end(ap);
    return retval;
}
