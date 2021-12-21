/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/fs.h>

#define LOG_TAG "cutils"

/* These defines are only needed because prebuilt headers are out of date */
#define __USE_XOPEN2K8 1
#define _ATFILE_SOURCE 1
#define _GNU_SOURCE 1

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <log/log.h>

#define ALL_PERMS (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO)
#define BUF_SIZE 64

static int fs_prepare_path_impl(const char* path, mode_t mode, uid_t uid, gid_t gid,
        int allow_fixup, int prepare_as_dir) {
    // TODO: fix the goto hell below.
    int type_ok;
    int owner_match;
    int mode_match;

    // Check if path needs to be created
    struct stat sb;
    int create_result = -1;
    if (TEMP_FAILURE_RETRY(lstat(path, &sb)) == -1) {
        if (errno == ENOENT) {
            goto create;
        } else {
            ALOGE("Failed to lstat(%s): %s", path, strerror(errno));
            return -1;
        }
    }

    // Exists, verify status
    type_ok = prepare_as_dir ? S_ISDIR(sb.st_mode) : S_ISREG(sb.st_mode);
    if (!type_ok) {
        ALOGE("Not a %s: %s", (prepare_as_dir ? "directory" : "regular file"), path);
        return -1;
    }

    owner_match = ((sb.st_uid == uid) && (sb.st_gid == gid));
    mode_match = ((sb.st_mode & ALL_PERMS) == mode);
    if (owner_match && mode_match) {
        return 0;
    } else if (allow_fixup) {
        goto fixup;
    } else {
        if (!owner_match) {
            ALOGE("Expected path %s with owner %d:%d but found %d:%d",
                    path, uid, gid, sb.st_uid, sb.st_gid);
            return -1;
        } else {
            ALOGW("Expected path %s with mode %o but found %o",
                    path, mode, (sb.st_mode & ALL_PERMS));
            return 0;
        }
    }

create:
    create_result = prepare_as_dir
        ? TEMP_FAILURE_RETRY(mkdir(path, mode))
        : TEMP_FAILURE_RETRY(open(path, O_CREAT | O_CLOEXEC | O_NOFOLLOW | O_RDONLY, 0644));
    if (create_result == -1) {
        if (errno != EEXIST) {
            ALOGE("Failed to %s(%s): %s",
                    (prepare_as_dir ? "mkdir" : "open"), path, strerror(errno));
            return -1;
        }
    } else if (!prepare_as_dir) {
        // For regular files we need to make sure we close the descriptor
        if (close(create_result) == -1) {
            ALOGW("Failed to close file after create %s: %s", path, strerror(errno));
        }
    }
fixup:
    if (TEMP_FAILURE_RETRY(chmod(path, mode)) == -1) {
        ALOGE("Failed to chmod(%s, %d): %s", path, mode, strerror(errno));
        return -1;
    }
    if (TEMP_FAILURE_RETRY(chown(path, uid, gid)) == -1) {
        ALOGE("Failed to chown(%s, %d, %d): %s", path, uid, gid, strerror(errno));
        return -1;
    }

    return 0;
}

int fs_prepare_dir(const char* path, mode_t mode, uid_t uid, gid_t gid) {
    return fs_prepare_path_impl(path, mode, uid, gid, /*allow_fixup*/ 1, /*prepare_as_dir*/ 1);
}

int fs_prepare_dir_strict(const char* path, mode_t mode, uid_t uid, gid_t gid) {
    return fs_prepare_path_impl(path, mode, uid, gid, /*allow_fixup*/ 0, /*prepare_as_dir*/ 1);
}

int fs_prepare_file_strict(const char* path, mode_t mode, uid_t uid, gid_t gid) {
    return fs_prepare_path_impl(path, mode, uid, gid, /*allow_fixup*/ 0, /*prepare_as_dir*/ 0);
}

int fs_read_atomic_int(const char* path, int* out_value) {
    int fd = TEMP_FAILURE_RETRY(open(path, O_RDONLY));
    if (fd == -1) {
        ALOGE("Failed to read %s: %s", path, strerror(errno));
        return -1;
    }

    char buf[BUF_SIZE];
    if (TEMP_FAILURE_RETRY(read(fd, buf, BUF_SIZE)) == -1) {
        ALOGE("Failed to read %s: %s", path, strerror(errno));
        goto fail;
    }
    if (sscanf(buf, "%d", out_value) != 1) {
        ALOGE("Failed to parse %s: %s", path, strerror(errno));
        goto fail;
    }
    close(fd);
    return 0;

fail:
    close(fd);
    *out_value = -1;
    return -1;
}

int fs_write_atomic_int(const char* path, int value) {
    char temp[PATH_MAX];
    if (snprintf(temp, PATH_MAX, "%s.XXXXXX", path) >= PATH_MAX) {
        ALOGE("Path too long");
        return -1;
    }

    int fd = TEMP_FAILURE_RETRY(mkstemp(temp));
    if (fd == -1) {
        ALOGE("Failed to open %s: %s", temp, strerror(errno));
        return -1;
    }

    char buf[BUF_SIZE];
    int len = snprintf(buf, BUF_SIZE, "%d", value) + 1;
    if (len > BUF_SIZE) {
        ALOGE("Value %d too large: %s", value, strerror(errno));
        goto fail;
    }
    if (TEMP_FAILURE_RETRY(write(fd, buf, len)) < len) {
        ALOGE("Failed to write %s: %s", temp, strerror(errno));
        goto fail;
    }
    if (close(fd) == -1) {
        ALOGE("Failed to close %s: %s", temp, strerror(errno));
        goto fail_closed;
    }

    if (rename(temp, path) == -1) {
        ALOGE("Failed to rename %s to %s: %s", temp, path, strerror(errno));
        goto fail_closed;
    }

    return 0;

fail:
    close(fd);
fail_closed:
    unlink(temp);
    return -1;
}

#ifndef __APPLE__

int fs_mkdirs(const char* path, mode_t mode) {
    if (*path != '/') {
        ALOGE("Relative paths are not allowed: %s", path);
        return -EINVAL;
    }

    int fd = open("/", 0);
    if (fd == -1) {
        ALOGE("Failed to open(/): %s", strerror(errno));
        return -errno;
    }

    struct stat sb;
    int res = 0;
    char* buf = strdup(path);
    char* segment = buf + 1;
    char* p = segment;
    while (*p != '\0') {
        if (*p == '/') {
            *p = '\0';

            if (!strcmp(segment, "..") || !strcmp(segment, ".") || !strcmp(segment, "")) {
                ALOGE("Invalid path: %s", buf);
                res = -EINVAL;
                goto done_close;
            }

            if (fstatat(fd, segment, &sb, AT_SYMLINK_NOFOLLOW) != 0) {
                if (errno == ENOENT) {
                    /* Nothing there yet; let's create it! */
                    if (mkdirat(fd, segment, mode) != 0) {
                        if (errno == EEXIST) {
                            /* We raced with someone; ignore */
                        } else {
                            ALOGE("Failed to mkdirat(%s): %s", buf, strerror(errno));
                            res = -errno;
                            goto done_close;
                        }
                    }
                } else {
                    ALOGE("Failed to fstatat(%s): %s", buf, strerror(errno));
                    res = -errno;
                    goto done_close;
                }
            } else {
                if (S_ISLNK(sb.st_mode)) {
                    ALOGE("Symbolic links are not allowed: %s", buf);
                    res = -ELOOP;
                    goto done_close;
                }
                if (!S_ISDIR(sb.st_mode)) {
                    ALOGE("Existing segment not a directory: %s", buf);
                    res = -ENOTDIR;
                    goto done_close;
                }
            }

            /* Yay, segment is ready for us to step into */
            int next_fd;
            if ((next_fd = openat(fd, segment, O_NOFOLLOW | O_CLOEXEC)) == -1) {
                ALOGE("Failed to openat(%s): %s", buf, strerror(errno));
                res = -errno;
                goto done_close;
            }

            close(fd);
            fd = next_fd;

            *p = '/';
            segment = p + 1;
        }
        p++;
    }

done_close:
    close(fd);
    free(buf);
    return res;
}

#endif
