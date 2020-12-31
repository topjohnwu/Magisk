/*
 * Host all missing/incomplete implementation in bionic
 * Copied from various sources
 * */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mntent.h>

#include "missing.hpp"

/* Original source: https://github.com/freebsd/freebsd/blob/master/contrib/file/src/getline.c
 * License: BSD, full copyright notice please check original source */

ssize_t compat_getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp) {
    char *ptr, *eptr;

    if (*buf == nullptr || *bufsiz == 0) {
        *bufsiz = BUFSIZ;
        if ((*buf = (char *) malloc(*bufsiz)) == nullptr)
            return -1;
    }

    for (ptr = *buf, eptr = *buf + *bufsiz;;) {
        int c = fgetc(fp);
        if (c == -1) {
            if (feof(fp))
                return ptr == *buf ? -1 : ptr - *buf;
            else
                return -1;
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
            if ((nbuf = (char *) realloc(*buf, nbufsiz)) == nullptr)
                return -1;
            *buf = nbuf;
            *bufsiz = nbufsiz;
            eptr = nbuf + nbufsiz;
            ptr = nbuf + d;
        }
    }
}

ssize_t compat_getline(char **buf, size_t *bufsiz, FILE *fp) {
    return getdelim(buf, bufsiz, '\n', fp);
}

/* Original source: https://android.googlesource.com/platform/bionic/+/master/libc/bionic/mntent.cpp
 * License: AOSP, full copyright notice please check original source */

struct mntent *compat_getmntent_r(FILE* fp, struct mntent* e, char* buf, int buf_len) {
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

FILE *compat_setmntent(const char* path, const char* mode) {
    return fopen(path, mode);
}

int compat_endmntent(FILE* fp) {
    if (fp != nullptr) {
        fclose(fp);
    }
    return 1;
}

char *compat_hasmntopt(const struct mntent* mnt, const char* opt) {
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

