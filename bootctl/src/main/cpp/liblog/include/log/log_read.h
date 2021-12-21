/*
 * Copyright (C) 2005-2017 The Android Open Source Project
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

#pragma once

#include <stdint.h>
#include <sys/types.h>

#include <android/log.h>
#include <log/log_time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ANDROID_LOG_WRAP_DEFAULT_TIMEOUT 7200 /* 2 hour default */

/*
 * Native log reading interface section. See logcat for sample code.
 *
 * The preferred API is an exec of logcat. Likely uses of this interface
 * are if native code suffers from exec or filtration being too costly,
 * access to raw information, or parsing is an issue.
 */

struct logger_entry {
  uint16_t len;      /* length of the payload */
  uint16_t hdr_size; /* sizeof(struct logger_entry) */
  int32_t pid;       /* generating process's pid */
  uint32_t tid;      /* generating process's tid */
  uint32_t sec;      /* seconds since Epoch */
  uint32_t nsec;     /* nanoseconds */
  uint32_t lid;      /* log id of the payload, bottom 4 bits currently */
  uint32_t uid;      /* generating process's uid */
};

/*
 * The maximum size of a log entry which can be read.
 * An attempt to read less than this amount may result
 * in read() returning EINVAL.
 */
#define LOGGER_ENTRY_MAX_LEN (5 * 1024)

struct log_msg {
  union {
    unsigned char buf[LOGGER_ENTRY_MAX_LEN + 1];
    struct logger_entry entry;
  } __attribute__((aligned(4)));
#ifdef __cplusplus
  uint64_t nsec() const {
    return static_cast<uint64_t>(entry.sec) * NS_PER_SEC + entry.nsec;
  }
  log_id_t id() {
    return static_cast<log_id_t>(entry.lid);
  }
  char* msg() {
    unsigned short hdr_size = entry.hdr_size;
    if (hdr_size >= sizeof(struct log_msg) - sizeof(entry)) {
      return nullptr;
    }
    return reinterpret_cast<char*>(buf) + hdr_size;
  }
  unsigned int len() { return entry.hdr_size + entry.len; }
#endif
};

struct logger;

log_id_t android_logger_get_id(struct logger* logger);

/* Clears the given log buffer. */
int android_logger_clear(struct logger* logger);
/* Return the allotted size for the given log buffer. */
long android_logger_get_log_size(struct logger* logger);
/* Set the allotted size for the given log buffer. */
int android_logger_set_log_size(struct logger* logger, unsigned long size);
/* Return the actual, uncompressed size that can be read from the given log buffer. */
long android_logger_get_log_readable_size(struct logger* logger);
/* Return the actual, compressed size that the given log buffer is consuming. */
long android_logger_get_log_consumed_size(struct logger* logger);
/* Deprecated.  Always returns '4' regardless of input. */
int android_logger_get_log_version(struct logger* logger);

struct logger_list;

ssize_t android_logger_get_statistics(struct logger_list* logger_list,
                                      char* buf, size_t len);
ssize_t android_logger_get_prune_list(struct logger_list* logger_list,
                                      char* buf, size_t len);
int android_logger_set_prune_list(struct logger_list* logger_list, const char* buf, size_t len);

/* The below values are used for the `mode` argument of the below functions. */
/* Note that 0x00000003 were previously used and should be considered reserved. */
#define ANDROID_LOG_NONBLOCK 0x00000800
#define ANDROID_LOG_WRAP 0x40000000 /* Block until buffer about to wrap */
#define ANDROID_LOG_PSTORE 0x80000000

struct logger_list* android_logger_list_alloc(int mode, unsigned int tail,
                                              pid_t pid);
struct logger_list* android_logger_list_alloc_time(int mode, log_time start,
                                                   pid_t pid);
void android_logger_list_free(struct logger_list* logger_list);
/* In the purest sense, the following two are orthogonal interfaces */
int android_logger_list_read(struct logger_list* logger_list,
                             struct log_msg* log_msg);

/* Multiple log_id_t opens */
struct logger* android_logger_open(struct logger_list* logger_list, log_id_t id);
/* Single log_id_t open */
struct logger_list* android_logger_list_open(log_id_t id, int mode,
                                             unsigned int tail, pid_t pid);
#define android_logger_list_close android_logger_list_free

#ifdef __cplusplus
}
#endif
