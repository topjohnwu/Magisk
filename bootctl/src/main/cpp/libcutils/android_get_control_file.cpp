/*
 * Copyright (C) 2016 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <cutils/android_get_control_file.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include <android-base/file.h>
#include <android-base/stringprintf.h>

#include "android_get_control_env.h"

int __android_get_control_from_env(const char* prefix, const char* name) {
    if (!prefix || !name) return -1;

    char *key = NULL;
    if (asprintf(&key, "%s%s", prefix, name) < 0) return -1;
    if (!key) return -1;

    char *cp = key;
    while (*cp) {
        if (!isalnum(*cp)) *cp = '_';
        ++cp;
    }

    const char* val = getenv(key);
    free(key);
    if (!val) return -1;

    errno = 0;
    long fd = strtol(val, NULL, 10);
    if (errno) return -1;

    // Since we are inheriting an fd, it could legitimately exceed _SC_OPEN_MAX
    if ((fd < 0) || (fd > INT_MAX)) return -1;

    // Still open?
    if (TEMP_FAILURE_RETRY(fcntl(fd, F_GETFD)) < 0) return -1;

    return static_cast<int>(fd);
}

int android_get_control_file(const char* path) {
    std::string given_path;
    if (!android::base::Realpath(path, &given_path)) return -1;

    // Try path, then realpath(path), as keys to get the fd from env.
    auto fd = __android_get_control_from_env(ANDROID_FILE_ENV_PREFIX, path);
    if (fd < 0) {
        fd = __android_get_control_from_env(ANDROID_FILE_ENV_PREFIX, given_path.c_str());
        if (fd < 0) return fd;
    }

    // Find file path from /proc and make sure it is correct
    auto proc = android::base::StringPrintf("/proc/self/fd/%d", fd);
    std::string fd_path;
    if (!android::base::Realpath(proc, &fd_path)) return -1;

    if (given_path != fd_path) return -1;
    // It is what we think it is

    return fd;
}
