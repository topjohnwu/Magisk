/*
 * Copyright (C) 2005-2016 The Android Open Source Project
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

#include <errno.h>
#include <stdint.h>

#ifdef __cplusplus
#include <string>
#endif

#include <log/log.h>

#ifdef __cplusplus
extern "C" {
#endif

/* For manipulating lists of events. */

#define ANDROID_MAX_LIST_NEST_DEPTH 8

/*
 * The opaque context used to manipulate lists of events.
 */
typedef struct android_log_context_internal* android_log_context;

/*
 * Elements returned when reading a list of events.
 */
typedef struct {
  AndroidEventLogType type;
  uint16_t complete;
  uint16_t len;
  union {
    int32_t int32;
    int64_t int64;
    char* string;
    float float32;
  } data;
} android_log_list_element;

/*
 * Creates a context associated with an event tag to write elements to
 * the list of events.
 */
android_log_context create_android_logger(uint32_t tag);

/* All lists must be braced by a begin and end call */
/*
 * NB: If the first level braces are missing when specifying multiple
 *     elements, we will manufacturer a list to embrace it for your API
 *     convenience. For a single element, it will remain solitary.
 */
int android_log_write_list_begin(android_log_context ctx);
int android_log_write_list_end(android_log_context ctx);

int android_log_write_int32(android_log_context ctx, int32_t value);
int android_log_write_int64(android_log_context ctx, int64_t value);
int android_log_write_string8(android_log_context ctx, const char* value);
int android_log_write_string8_len(android_log_context ctx, const char* value,
                                  size_t maxlen);
int android_log_write_float32(android_log_context ctx, float value);

/* Submit the composed list context to the specified logger id */
/* NB: LOG_ID_EVENTS and LOG_ID_SECURITY only valid binary buffers */
int android_log_write_list(android_log_context ctx, log_id_t id);

/*
 * Creates a context from a raw buffer representing a list of events to be read.
 */
android_log_context create_android_log_parser(const char* msg, size_t len);

android_log_list_element android_log_read_next(android_log_context ctx);
android_log_list_element android_log_peek_next(android_log_context ctx);

/* Reset writer context */
int android_log_reset(android_log_context ctx);

/* Reset reader context */
int android_log_parser_reset(android_log_context ctx,
                             const char* msg, size_t len);

/* Finished with reader or writer context */
int android_log_destroy(android_log_context* ctx);

#ifdef __cplusplus
/* android_log_list C++ helpers */
extern "C++" {
class android_log_event_list {
 private:
  android_log_context ctx;
  int ret;

  android_log_event_list(const android_log_event_list&) = delete;
  void operator=(const android_log_event_list&) = delete;

 public:
  explicit android_log_event_list(int tag) : ret(0) {
    ctx = create_android_logger(static_cast<uint32_t>(tag));
  }
  ~android_log_event_list() {
    android_log_destroy(&ctx);
  }

  int close() {
    int retval = android_log_destroy(&ctx);
    if (retval < 0) ret = retval;
    return retval;
  }

  /* To allow above C calls to use this class as parameter */
  operator android_log_context() const {
    return ctx;
  }

  /* return errors or transmit status */
  int status() const {
    return ret;
  }

  int begin() {
    int retval = android_log_write_list_begin(ctx);
    if (retval < 0) ret = retval;
    return ret;
  }
  int end() {
    int retval = android_log_write_list_end(ctx);
    if (retval < 0) ret = retval;
    return ret;
  }

  android_log_event_list& operator<<(int32_t value) {
    int retval = android_log_write_int32(ctx, value);
    if (retval < 0) ret = retval;
    return *this;
  }

  android_log_event_list& operator<<(uint32_t value) {
    int retval = android_log_write_int32(ctx, static_cast<int32_t>(value));
    if (retval < 0) ret = retval;
    return *this;
  }

  android_log_event_list& operator<<(bool value) {
    int retval = android_log_write_int32(ctx, value ? 1 : 0);
    if (retval < 0) ret = retval;
    return *this;
  }

  android_log_event_list& operator<<(int64_t value) {
    int retval = android_log_write_int64(ctx, value);
    if (retval < 0) ret = retval;
    return *this;
  }

  android_log_event_list& operator<<(uint64_t value) {
    int retval = android_log_write_int64(ctx, static_cast<int64_t>(value));
    if (retval < 0) ret = retval;
    return *this;
  }

  android_log_event_list& operator<<(const char* value) {
    int retval = android_log_write_string8(ctx, value);
    if (retval < 0) ret = retval;
    return *this;
  }

  android_log_event_list& operator<<(const std::string& value) {
    int retval =
        android_log_write_string8_len(ctx, value.data(), value.length());
    if (retval < 0) ret = retval;
    return *this;
  }

  android_log_event_list& operator<<(float value) {
    int retval = android_log_write_float32(ctx, value);
    if (retval < 0) ret = retval;
    return *this;
  }

  int write(log_id_t id = LOG_ID_EVENTS) {
    /* facilitate -EBUSY retry */
    if ((ret == -EBUSY) || (ret > 0)) ret = 0;
    int retval = android_log_write_list(ctx, id);
    /* existing errors trump transmission errors */
    if (!ret) ret = retval;
    return ret;
  }

  int operator<<(log_id_t id) {
    write(id);
    android_log_destroy(&ctx);
    return ret;
  }

  /*
   * Append<Type> methods removes any integer promotion
   * confusion, and adds access to string with length.
   * Append methods are also added for all types for
   * convenience.
   */

  bool AppendInt(int32_t value) {
    int retval = android_log_write_int32(ctx, value);
    if (retval < 0) ret = retval;
    return ret >= 0;
  }

  bool AppendLong(int64_t value) {
    int retval = android_log_write_int64(ctx, value);
    if (retval < 0) ret = retval;
    return ret >= 0;
  }

  bool AppendString(const char* value) {
    int retval = android_log_write_string8(ctx, value);
    if (retval < 0) ret = retval;
    return ret >= 0;
  }

  bool AppendString(const char* value, size_t len) {
    int retval = android_log_write_string8_len(ctx, value, len);
    if (retval < 0) ret = retval;
    return ret >= 0;
  }

  bool AppendString(const std::string& value) {
    int retval =
        android_log_write_string8_len(ctx, value.data(), value.length());
    if (retval < 0) ret = retval;
    return ret;
  }

  bool Append(const std::string& value) {
    int retval =
        android_log_write_string8_len(ctx, value.data(), value.length());
    if (retval < 0) ret = retval;
    return ret;
  }

  bool AppendFloat(float value) {
    int retval = android_log_write_float32(ctx, value);
    if (retval < 0) ret = retval;
    return ret >= 0;
  }

  template <typename Tvalue>
  bool Append(Tvalue value) {
    *this << value;
    return ret >= 0;
  }

  bool Append(const char* value, size_t len) {
    int retval = android_log_write_string8_len(ctx, value, len);
    if (retval < 0) ret = retval;
    return ret >= 0;
  }
};
}
#endif

#ifdef __cplusplus
}
#endif
