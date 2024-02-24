#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

// Source: bionic/libc/bionic/libgen.cpp
static int __basename_r(const char *path, char* buffer, size_t buffer_size) {
    const char *startp = NULL;
    const char *endp = NULL;
    int len;
    int result;

    // Empty or NULL string gets treated as ".".
    if (path == NULL || *path == '\0') {
        startp = ".";
        len = 1;
        goto Exit;
    }

    // Strip trailing slashes.
    endp = path + strlen(path) - 1;
    while (endp > path && *endp == '/') {
        endp--;
    }

    // All slashes becomes "/".
    if (endp == path && *endp == '/') {
        startp = "/";
        len = 1;
        goto Exit;
    }

    // Find the start of the base.
    startp = endp;
    while (startp > path && *(startp - 1) != '/') {
        startp--;
    }

    len = endp - startp +1;

Exit:
    result = len;
    if (buffer == NULL) {
        return result;
    }
    if (len > (int) buffer_size - 1) {
        len = buffer_size - 1;
        result = -1;
        errno = ERANGE;
    }

    if (len >= 0) {
        memcpy(buffer, startp, len);
        buffer[len] = 0;
    }
    return result;
}

char *basename(const char *path) {
    static char buf[4069];
    int rc = __basename_r(path, buf, sizeof(buf));
    return (rc < 0) ? NULL : buf;
}
