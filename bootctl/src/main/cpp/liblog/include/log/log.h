/*
 * Copyright (C) 2005-2014 The Android Open Source Project
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

/* Too many in the ecosystem assume these are included */
#if !defined(_WIN32)
#include <pthread.h>
#endif
#include <stdint.h> /* uint16_t, int32_t */
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <android/log.h>
#include <log/log_id.h>
#include <log/log_main.h>
#include <log/log_radio.h>
#include <log/log_safetynet.h>
#include <log/log_system.h>
#include <log/log_time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * LOG_TAG is the local tag used for the following simplified
 * logging macros.  You can change this preprocessor definition
 * before using the other macros to change the tag.
 */

#ifndef LOG_TAG
#define LOG_TAG NULL
#endif

/*
 * Normally we strip the effects of ALOGV (VERBOSE messages),
 * LOG_FATAL and LOG_FATAL_IF (FATAL assert messages) from the
 * release builds be defining NDEBUG.  You can modify this (for
 * example with "#define LOG_NDEBUG 0" at the top of your source
 * file) to change that behavior.
 */

#ifndef LOG_NDEBUG
#ifdef NDEBUG
#define LOG_NDEBUG 1
#else
#define LOG_NDEBUG 0
#endif
#endif

/*
 * The maximum size of the log entry payload that can be
 * written to the logger. An attempt to write more than
 * this amount will result in a truncated log entry.
 */
#define LOGGER_ENTRY_MAX_PAYLOAD 4068

/*
 * Event logging.
 */

/*
 * The following should not be used directly.
 */

int __android_log_bwrite(int32_t tag, const void* payload, size_t len);
int __android_log_btwrite(int32_t tag, char type, const void* payload,
                          size_t len);
int __android_log_bswrite(int32_t tag, const char* payload);

int __android_log_stats_bwrite(int32_t tag, const void* payload, size_t len);

#define android_bWriteLog(tag, payload, len) \
  __android_log_bwrite(tag, payload, len)
#define android_btWriteLog(tag, type, payload, len) \
  __android_log_btwrite(tag, type, payload, len)

/*
 * Event log entry types.
 */
typedef enum {
  /* Special markers for android_log_list_element type */
  EVENT_TYPE_LIST_STOP = '\n', /* declare end of list  */
  EVENT_TYPE_UNKNOWN = '?',    /* protocol error       */

  /* must match with declaration in java/android/android/util/EventLog.java */
  EVENT_TYPE_INT = 0,  /* int32_t */
  EVENT_TYPE_LONG = 1, /* int64_t */
  EVENT_TYPE_STRING = 2,
  EVENT_TYPE_LIST = 3,
  EVENT_TYPE_FLOAT = 4,
} AndroidEventLogType;

#ifndef LOG_EVENT_INT
#define LOG_EVENT_INT(_tag, _value)                                          \
  {                                                                          \
    int intBuf = _value;                                                     \
    (void)android_btWriteLog(_tag, EVENT_TYPE_INT, &intBuf, sizeof(intBuf)); \
  }
#endif
#ifndef LOG_EVENT_LONG
#define LOG_EVENT_LONG(_tag, _value)                                            \
  {                                                                             \
    long long longBuf = _value;                                                 \
    (void)android_btWriteLog(_tag, EVENT_TYPE_LONG, &longBuf, sizeof(longBuf)); \
  }
#endif
#ifndef LOG_EVENT_FLOAT
#define LOG_EVENT_FLOAT(_tag, _value)                           \
  {                                                             \
    float floatBuf = _value;                                    \
    (void)android_btWriteLog(_tag, EVENT_TYPE_FLOAT, &floatBuf, \
                             sizeof(floatBuf));                 \
  }
#endif
#ifndef LOG_EVENT_STRING
#define LOG_EVENT_STRING(_tag, _value) \
  (void)__android_log_bswrite(_tag, _value);
#endif

/* --------------------------------------------------------------------- */

/*
 * Release any logger resources (a new log write will immediately re-acquire)
 *
 * This is specifically meant to be used by Zygote to close open file descriptors after fork()
 * and before specialization.  O_CLOEXEC is used on file descriptors, so they will be closed upon
 * exec() in normal use cases.
 *
 * Note that this is not safe to call from a multi-threaded program.
 */
void __android_log_close(void);

#ifdef __cplusplus
}
#endif
