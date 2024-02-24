/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <dirent.h>

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "private/ErrnoRestorer.h"

extern "C" int sys_getdents64(unsigned int, dirent*, unsigned int);

// Apportable decided to copy the data structure from this file
// and use it in their own code, but they also call into readdir.
// In order to avoid a lockup, the structure must be maintained in
// the exact same order as in L and below. New structure members
// need to be added to the end of this structure.
// See b/21037208 for more details.
struct DIR {
  int fd_;
  size_t available_bytes_;
  dirent* next_;
  dirent buff_[15];
  long current_pos_;
};

static DIR* __allocate_DIR(int fd) {
  DIR* d = reinterpret_cast<DIR*>(malloc(sizeof(DIR)));
  if (d == nullptr) {
    return nullptr;
  }
  d->fd_ = fd;
  d->available_bytes_ = 0;
  d->next_ = nullptr;
  d->current_pos_ = 0L;
  return d;
}

int dirfd(DIR* d) {
  return d->fd_;
}

DIR* fdopendir(int fd) {
  // Is 'fd' actually a directory?
  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    return nullptr;
  }
  if (!S_ISDIR(sb.st_mode)) {
    errno = ENOTDIR;
    return nullptr;
  }

  return __allocate_DIR(fd);
}

DIR* opendir(const char* path) {
  int fd = open(path, O_CLOEXEC | O_DIRECTORY | O_RDONLY);
  return (fd != -1) ? __allocate_DIR(fd) : nullptr;
}

static bool __fill_DIR(DIR* d) {
  int rc = TEMP_FAILURE_RETRY(sys_getdents64(d->fd_, d->buff_, sizeof(d->buff_)));
  if (rc <= 0) {
    return false;
  }
  d->available_bytes_ = rc;
  d->next_ = d->buff_;
  return true;
}

static dirent* __readdir_locked(DIR* d) {
  if (d->available_bytes_ == 0 && !__fill_DIR(d)) {
    return nullptr;
  }

  dirent* entry = d->next_;
  d->next_ = reinterpret_cast<dirent*>(reinterpret_cast<char*>(entry) + entry->d_reclen);
  d->available_bytes_ -= entry->d_reclen;
  // The directory entry offset uses 0, 1, 2 instead of real file offset,
  // so the value range of long type is enough.
  d->current_pos_ = static_cast<long>(entry->d_off);
  return entry;
}

dirent* readdir(DIR* d) {
  return __readdir_locked(d);
}
__strong_alias(readdir64, readdir);

int readdir_r(DIR* d, dirent* entry, dirent** result) {
  ErrnoRestorer errno_restorer;

  *result = nullptr;
  errno = 0;

  dirent* next = __readdir_locked(d);
  if (errno != 0 && next == nullptr) {
    return errno;
  }

  if (next != nullptr) {
    memcpy(entry, next, next->d_reclen);
    *result = entry;
  }
  return 0;
}
__strong_alias(readdir64_r, readdir_r);

int closedir(DIR* d) {
  if (d == nullptr) {
    errno = EINVAL;
    return -1;
  }

  int fd = d->fd_;
  int rc = close(fd);
  free(d);
  return rc;
}

void rewinddir(DIR* d) {
  lseek(d->fd_, 0, SEEK_SET);
  d->available_bytes_ = 0;
  d->current_pos_ = 0L;
}

void seekdir(DIR* d, long offset) {
  off_t ret = lseek(d->fd_, offset, SEEK_SET);
  if (ret != -1L) {
    d->available_bytes_ = 0;
    d->current_pos_ = ret;
  }
}

long telldir(DIR* d) {
  return d->current_pos_;
}
