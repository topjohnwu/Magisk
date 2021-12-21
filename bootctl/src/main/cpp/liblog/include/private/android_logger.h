/*
 * Copyright (C) 2015 The Android Open Source Project
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

/* This file is used to define the internal protocol for the Android Logger */

#pragma once

/* Android private interfaces */

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
#include <string>
#endif

#include <log/log.h>
#include <log/log_event_list.h>

#define LOGGER_MAGIC 'l'

#if defined(__cplusplus)
extern "C" {
#endif

/* Header Structure to pstore */
typedef struct __attribute__((__packed__)) {
  uint8_t magic;
  uint16_t len;
  uint16_t uid;
  uint16_t pid;
} android_pmsg_log_header_t;

/* Header Structure to logd, and second header for pstore */
typedef struct __attribute__((__packed__)) {
  uint8_t id;
  uint16_t tid;
  log_time realtime;
} android_log_header_t;

/* Event Header Structure to logd */
typedef struct __attribute__((__packed__)) {
  int32_t tag;  // Little Endian Order
} android_event_header_t;

// Event payload EVENT_TYPE_LIST
typedef struct __attribute__((__packed__)) {
  int8_t type;  // EVENT_TYPE_LIST
  int8_t element_count;
} android_event_list_t;

// Event payload EVENT_TYPE_FLOAT
typedef struct __attribute__((__packed__)) {
  int8_t type;  // EVENT_TYPE_FLOAT
  float data;
} android_event_float_t;

/* Event payload EVENT_TYPE_INT */
typedef struct __attribute__((__packed__)) {
  int8_t type;   // EVENT_TYPE_INT
  int32_t data;  // Little Endian Order
} android_event_int_t;

/* Event with single EVENT_TYPE_INT */
typedef struct __attribute__((__packed__)) {
  android_event_header_t header;
  android_event_int_t payload;
} android_log_event_int_t;

/* Event payload EVENT_TYPE_LONG */
typedef struct __attribute__((__packed__)) {
  int8_t type;   // EVENT_TYPE_LONG
  int64_t data;  // Little Endian Order
} android_event_long_t;

/* Event with single EVENT_TYPE_LONG */
typedef struct __attribute__((__packed__)) {
  android_event_header_t header;
  android_event_long_t payload;
} android_log_event_long_t;

/*
 * Event payload EVENT_TYPE_STRING
 *
 * Danger: do not embed this structure into another structure.
 * This structure uses a flexible array member, and when
 * compiled using g++, __builtin_object_size(data, 1) returns
 * a bad value. This is possibly a g++ bug, or a bug due to
 * the fact that flexible array members are not supported
 * in C++.
 * http://stackoverflow.com/questions/4412749/are-flexible-array-members-valid-in-c
 */

typedef struct __attribute__((__packed__)) {
  int8_t type;     // EVENT_TYPE_STRING;
  int32_t length;  // Little Endian Order
  char data[];
} android_event_string_t;

/* Event with single EVENT_TYPE_STRING */
typedef struct __attribute__((__packed__)) {
  android_event_header_t header;
  int8_t type;     // EVENT_TYPE_STRING;
  int32_t length;  // Little Endian Order
  char data[];
} android_log_event_string_t;

#define ANDROID_LOG_PMSG_FILE_MAX_SEQUENCE 256 /* 1MB file */
#define ANDROID_LOG_PMSG_FILE_SEQUENCE 1000

ssize_t __android_log_pmsg_file_write(log_id_t logId, char prio,
                                      const char* filename, const char* buf,
                                      size_t len);

#define LOG_ID_ANY ((log_id_t)-1)
#define ANDROID_LOG_ANY ANDROID_LOG_UNKNOWN

/* first 5 arguments match __android_log_msg_file_write, a cast is safe */
typedef ssize_t (*__android_log_pmsg_file_read_fn)(log_id_t logId, char prio,
                                                   const char* filename,
                                                   const char* buf, size_t len,
                                                   void* arg);

ssize_t __android_log_pmsg_file_read(log_id_t logId, char prio,
                                     const char* prefix,
                                     __android_log_pmsg_file_read_fn fn,
                                     void* arg);

int __android_log_security_bwrite(int32_t tag, const void* payload, size_t len);
int __android_log_security_bswrite(int32_t tag, const char* payload);
int __android_log_security(); /* Device Owner is present */

/* Retrieve the composed event buffer */
int android_log_write_list_buffer(android_log_context ctx, const char** msg);

#if defined(__cplusplus)
}
#endif
