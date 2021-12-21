/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <log/log_event_list.h>
#include <private/android_logger.h>

#define MAX_EVENT_PAYLOAD (LOGGER_ENTRY_MAX_PAYLOAD - sizeof(int32_t))

enum ReadWriteFlag {
  kAndroidLoggerRead = 1,
  kAndroidLoggerWrite = 2,
};

struct android_log_context_internal {
  uint32_t tag;
  unsigned pos;                                    /* Read/write position into buffer */
  unsigned count[ANDROID_MAX_LIST_NEST_DEPTH + 1]; /* Number of elements   */
  unsigned list[ANDROID_MAX_LIST_NEST_DEPTH + 1];  /* pos for list counter */
  unsigned list_nest_depth;
  unsigned len; /* Length or raw buffer. */
  bool overflow;
  bool list_stop; /* next call decrement list_nest_depth and issue a stop */
  ReadWriteFlag read_write_flag;
  uint8_t storage[LOGGER_ENTRY_MAX_PAYLOAD];
};

static void init_context(android_log_context_internal* context, uint32_t tag) {
  context->tag = tag;
  context->read_write_flag = kAndroidLoggerWrite;
  size_t needed = sizeof(android_event_list_t);
  if ((context->pos + needed) > MAX_EVENT_PAYLOAD) {
    context->overflow = true;
  }
  /* Everything is a list */
  context->storage[context->pos + 0] = EVENT_TYPE_LIST;
  context->list[0] = context->pos + 1;
  context->pos += needed;
}

static void init_parser_context(android_log_context_internal* context, const char* msg,
                                size_t len) {
  len = (len <= MAX_EVENT_PAYLOAD) ? len : MAX_EVENT_PAYLOAD;
  context->len = len;
  memcpy(context->storage, msg, len);
  context->read_write_flag = kAndroidLoggerRead;
}

android_log_context create_android_logger(uint32_t tag) {
  android_log_context_internal* context;

  context =
      static_cast<android_log_context_internal*>(calloc(1, sizeof(android_log_context_internal)));
  if (!context) {
    return NULL;
  }
  init_context(context, tag);

  return (android_log_context)context;
}

android_log_context create_android_log_parser(const char* msg, size_t len) {
  android_log_context_internal* context;

  context =
      static_cast<android_log_context_internal*>(calloc(1, sizeof(android_log_context_internal)));
  if (!context) {
    return NULL;
  }
  init_parser_context(context, msg, len);

  return (android_log_context)context;
}

int android_log_destroy(android_log_context* ctx) {
  android_log_context_internal* context;

  context = (android_log_context_internal*)*ctx;
  if (!context) {
    return -EBADF;
  }
  memset(context, 0, sizeof(*context));
  free(context);
  *ctx = NULL;
  return 0;
}

int android_log_reset(android_log_context context) {
  uint32_t tag;

  if (!context || (kAndroidLoggerWrite != context->read_write_flag)) {
    return -EBADF;
  }

  tag = context->tag;
  memset(context, 0, sizeof(*context));
  init_context(context, tag);

  return 0;
}

int android_log_parser_reset(android_log_context context, const char* msg, size_t len) {
  if (!context || (kAndroidLoggerRead != context->read_write_flag)) {
    return -EBADF;
  }

  memset(context, 0, sizeof(*context));
  init_parser_context(context, msg, len);

  return 0;
}

int android_log_write_list_begin(android_log_context context) {
  if (!context || (kAndroidLoggerWrite != context->read_write_flag)) {
    return -EBADF;
  }
  if (context->list_nest_depth > ANDROID_MAX_LIST_NEST_DEPTH) {
    context->overflow = true;
    return -EOVERFLOW;
  }
  size_t needed = sizeof(android_event_list_t);
  if ((context->pos + needed) > MAX_EVENT_PAYLOAD) {
    context->overflow = true;
    return -EIO;
  }
  context->count[context->list_nest_depth]++;
  context->list_nest_depth++;
  if (context->list_nest_depth > ANDROID_MAX_LIST_NEST_DEPTH) {
    context->overflow = true;
    return -EOVERFLOW;
  }
  if (context->overflow) {
    return -EIO;
  }
  auto* event_list = reinterpret_cast<android_event_list_t*>(&context->storage[context->pos]);
  event_list->type = EVENT_TYPE_LIST;
  event_list->element_count = 0;
  context->list[context->list_nest_depth] = context->pos + 1;
  context->count[context->list_nest_depth] = 0;
  context->pos += needed;
  return 0;
}

int android_log_write_int32(android_log_context context, int32_t value) {
  if (!context || (kAndroidLoggerWrite != context->read_write_flag)) {
    return -EBADF;
  }
  if (context->overflow) {
    return -EIO;
  }
  size_t needed = sizeof(android_event_int_t);
  if ((context->pos + needed) > MAX_EVENT_PAYLOAD) {
    context->overflow = true;
    return -EIO;
  }
  context->count[context->list_nest_depth]++;
  auto* event_int = reinterpret_cast<android_event_int_t*>(&context->storage[context->pos]);
  event_int->type = EVENT_TYPE_INT;
  event_int->data = value;
  context->pos += needed;
  return 0;
}

int android_log_write_int64(android_log_context context, int64_t value) {
  if (!context || (kAndroidLoggerWrite != context->read_write_flag)) {
    return -EBADF;
  }
  if (context->overflow) {
    return -EIO;
  }
  size_t needed = sizeof(android_event_long_t);
  if ((context->pos + needed) > MAX_EVENT_PAYLOAD) {
    context->overflow = true;
    return -EIO;
  }
  context->count[context->list_nest_depth]++;
  auto* event_long = reinterpret_cast<android_event_long_t*>(&context->storage[context->pos]);
  event_long->type = EVENT_TYPE_LONG;
  event_long->data = value;
  context->pos += needed;
  return 0;
}

int android_log_write_string8_len(android_log_context context, const char* value, size_t maxlen) {
  if (!context || (kAndroidLoggerWrite != context->read_write_flag)) {
    return -EBADF;
  }
  if (context->overflow) {
    return -EIO;
  }
  if (!value) {
    value = "";
  }
  int32_t len = strnlen(value, maxlen);
  size_t needed = sizeof(android_event_string_t) + len;
  if ((context->pos + needed) > MAX_EVENT_PAYLOAD) {
    /* Truncate string for delivery */
    len = MAX_EVENT_PAYLOAD - context->pos - 1 - sizeof(int32_t);
    if (len <= 0) {
      context->overflow = true;
      return -EIO;
    }
  }
  context->count[context->list_nest_depth]++;
  auto* event_string = reinterpret_cast<android_event_string_t*>(&context->storage[context->pos]);
  event_string->type = EVENT_TYPE_STRING;
  event_string->length = len;
  if (len) {
    memcpy(&event_string->data, value, len);
  }
  context->pos += needed;
  return len;
}

int android_log_write_string8(android_log_context ctx, const char* value) {
  return android_log_write_string8_len(ctx, value, MAX_EVENT_PAYLOAD);
}

int android_log_write_float32(android_log_context context, float value) {
  if (!context || (kAndroidLoggerWrite != context->read_write_flag)) {
    return -EBADF;
  }
  if (context->overflow) {
    return -EIO;
  }
  size_t needed = sizeof(android_event_float_t);
  if ((context->pos + needed) > MAX_EVENT_PAYLOAD) {
    context->overflow = true;
    return -EIO;
  }
  context->count[context->list_nest_depth]++;
  auto* event_float = reinterpret_cast<android_event_float_t*>(&context->storage[context->pos]);
  event_float->type = EVENT_TYPE_FLOAT;
  event_float->data = value;
  context->pos += needed;
  return 0;
}

int android_log_write_list_end(android_log_context context) {
  if (!context || (kAndroidLoggerWrite != context->read_write_flag)) {
    return -EBADF;
  }
  if (context->list_nest_depth > ANDROID_MAX_LIST_NEST_DEPTH) {
    context->overflow = true;
    context->list_nest_depth--;
    return -EOVERFLOW;
  }
  if (!context->list_nest_depth) {
    context->overflow = true;
    return -EOVERFLOW;
  }
  if (context->list[context->list_nest_depth] <= 0) {
    context->list_nest_depth--;
    context->overflow = true;
    return -EOVERFLOW;
  }
  context->storage[context->list[context->list_nest_depth]] =
      context->count[context->list_nest_depth];
  context->list_nest_depth--;
  return 0;
}

/*
 * Logs the list of elements to the event log.
 */
int android_log_write_list(android_log_context context, log_id_t id) {
  const char* msg;
  ssize_t len;

  if ((id != LOG_ID_EVENTS) && (id != LOG_ID_SECURITY) && (id != LOG_ID_STATS)) {
    return -EINVAL;
  }

  if (!context || (kAndroidLoggerWrite != context->read_write_flag)) {
    return -EBADF;
  }
  if (context->list_nest_depth) {
    return -EIO;
  }
  /* NB: if there was overflow, then log is truncated. Nothing reported */
  context->storage[1] = context->count[0];
  len = context->len = context->pos;
  msg = (const char*)context->storage;
  /* it's not a list */
  if (context->count[0] <= 1) {
    len -= sizeof(uint8_t) + sizeof(uint8_t);
    if (len < 0) {
      len = 0;
    }
    msg += sizeof(uint8_t) + sizeof(uint8_t);
  }
  return (id == LOG_ID_EVENTS)
             ? __android_log_bwrite(context->tag, msg, len)
             : ((id == LOG_ID_STATS) ? __android_log_stats_bwrite(context->tag, msg, len)
                                     : __android_log_security_bwrite(context->tag, msg, len));
}

int android_log_write_list_buffer(android_log_context context, const char** buffer) {
  const char* msg;
  ssize_t len;

  if (!context || (kAndroidLoggerWrite != context->read_write_flag)) {
    return -EBADF;
  }
  if (context->list_nest_depth) {
    return -EIO;
  }
  if (buffer == NULL) {
    return -EFAULT;
  }
  /* NB: if there was overflow, then log is truncated. Nothing reported */
  context->storage[1] = context->count[0];
  len = context->len = context->pos;
  msg = (const char*)context->storage;
  /* it's not a list */
  if (context->count[0] <= 1) {
    len -= sizeof(uint8_t) + sizeof(uint8_t);
    if (len < 0) {
      len = 0;
    }
    msg += sizeof(uint8_t) + sizeof(uint8_t);
  }
  *buffer = msg;
  return len;
}

/*
 * Gets the next element. Parsing errors result in an EVENT_TYPE_UNKNOWN type.
 * If there is nothing to process, the complete field is set to non-zero. If
 * an EVENT_TYPE_UNKNOWN type is returned once, and the caller does not check
 * this and continues to call this function, the behavior is undefined
 * (although it won't crash).
 */
static android_log_list_element android_log_read_next_internal(android_log_context context,
                                                               int peek) {
  android_log_list_element elem;
  unsigned pos;

  memset(&elem, 0, sizeof(elem));

  /* Nothing to parse from this context, so return complete. */
  if (!context || (kAndroidLoggerRead != context->read_write_flag) ||
      (context->list_nest_depth > ANDROID_MAX_LIST_NEST_DEPTH) ||
      (context->count[context->list_nest_depth] >=
       (MAX_EVENT_PAYLOAD / (sizeof(uint8_t) + sizeof(uint8_t))))) {
    elem.type = EVENT_TYPE_UNKNOWN;
    if (context &&
        (context->list_stop || ((context->list_nest_depth <= ANDROID_MAX_LIST_NEST_DEPTH) &&
                                !context->count[context->list_nest_depth]))) {
      elem.type = EVENT_TYPE_LIST_STOP;
    }
    elem.complete = true;
    return elem;
  }

  /*
   * Use a different variable to update the position in case this
   * operation is a "peek".
   */
  pos = context->pos;
  if (context->list_stop) {
    elem.type = EVENT_TYPE_LIST_STOP;
    elem.complete = !context->count[0] && (!context->list_nest_depth ||
                                           ((context->list_nest_depth == 1) && !context->count[1]));
    if (!peek) {
      /* Suck in superfluous stop */
      if (context->storage[pos] == EVENT_TYPE_LIST_STOP) {
        context->pos = pos + 1;
      }
      if (context->list_nest_depth) {
        --context->list_nest_depth;
        if (context->count[context->list_nest_depth]) {
          context->list_stop = false;
        }
      } else {
        context->list_stop = false;
      }
    }
    return elem;
  }
  if ((pos + 1) > context->len) {
    elem.type = EVENT_TYPE_UNKNOWN;
    elem.complete = true;
    return elem;
  }

  elem.type = static_cast<AndroidEventLogType>(context->storage[pos]);
  switch ((int)elem.type) {
    case EVENT_TYPE_FLOAT:
    /* Rely on union to translate elem.data.int32 into elem.data.float32 */
    /* FALLTHRU */
    case EVENT_TYPE_INT: {
      elem.len = sizeof(int32_t);
      if ((pos + sizeof(android_event_int_t)) > context->len) {
        elem.type = EVENT_TYPE_UNKNOWN;
        return elem;
      }

      auto* event_int = reinterpret_cast<android_event_int_t*>(&context->storage[pos]);
      pos += sizeof(android_event_int_t);
      elem.data.int32 = event_int->data;
      /* common tangeable object suffix */
      elem.complete = !context->list_nest_depth && !context->count[0];
      if (!peek) {
        if (!context->count[context->list_nest_depth] ||
            !--(context->count[context->list_nest_depth])) {
          context->list_stop = true;
        }
        context->pos = pos;
      }
      return elem;
    }

    case EVENT_TYPE_LONG: {
      elem.len = sizeof(int64_t);
      if ((pos + sizeof(android_event_long_t)) > context->len) {
        elem.type = EVENT_TYPE_UNKNOWN;
        return elem;
      }

      auto* event_long = reinterpret_cast<android_event_long_t*>(&context->storage[pos]);
      pos += sizeof(android_event_long_t);
      elem.data.int64 = event_long->data;
      /* common tangeable object suffix */
      elem.complete = !context->list_nest_depth && !context->count[0];
      if (!peek) {
        if (!context->count[context->list_nest_depth] ||
            !--(context->count[context->list_nest_depth])) {
          context->list_stop = true;
        }
        context->pos = pos;
      }
      return elem;
    }

    case EVENT_TYPE_STRING: {
      if ((pos + sizeof(android_event_string_t)) > context->len) {
        elem.type = EVENT_TYPE_UNKNOWN;
        elem.complete = true;
        return elem;
      }
      auto* event_string = reinterpret_cast<android_event_string_t*>(&context->storage[pos]);
      pos += sizeof(android_event_string_t);
      // Wire format is int32_t, but elem.len is uint16_t...
      if (event_string->length >= UINT16_MAX) {
        elem.type = EVENT_TYPE_UNKNOWN;
        return elem;
      }
      elem.len = event_string->length;
      if ((pos + elem.len) > context->len) {
        elem.len = context->len - pos; /* truncate string */
        elem.complete = true;
        if (!elem.len) {
          elem.type = EVENT_TYPE_UNKNOWN;
          return elem;
        }
      }
      elem.data.string = event_string->data;
      /* common tangeable object suffix */
      pos += elem.len;
      elem.complete = !context->list_nest_depth && !context->count[0];
      if (!peek) {
        if (!context->count[context->list_nest_depth] ||
            !--(context->count[context->list_nest_depth])) {
          context->list_stop = true;
        }
        context->pos = pos;
      }
      return elem;
    }

    case EVENT_TYPE_LIST: {
      if ((pos + sizeof(android_event_list_t)) > context->len) {
        elem.type = EVENT_TYPE_UNKNOWN;
        elem.complete = true;
        return elem;
      }
      auto* event_list = reinterpret_cast<android_event_list_t*>(&context->storage[pos]);
      pos += sizeof(android_event_list_t);
      elem.complete = context->list_nest_depth >= ANDROID_MAX_LIST_NEST_DEPTH;
      if (peek) {
        return elem;
      }
      if (context->count[context->list_nest_depth]) {
        context->count[context->list_nest_depth]--;
      }
      context->list_stop = event_list->element_count == 0;
      context->list_nest_depth++;
      if (context->list_nest_depth <= ANDROID_MAX_LIST_NEST_DEPTH) {
        context->count[context->list_nest_depth] = event_list->element_count;
      }
      context->pos = pos;
      return elem;
    }

    case EVENT_TYPE_LIST_STOP: /* Suprise Newline terminates lists. */
      pos++;
      if (!peek) {
        context->pos = pos;
      }
      elem.type = EVENT_TYPE_UNKNOWN;
      elem.complete = !context->list_nest_depth;
      if (context->list_nest_depth > 0) {
        elem.type = EVENT_TYPE_LIST_STOP;
        if (!peek) {
          context->list_nest_depth--;
        }
      }
      return elem;

    default:
      elem.type = EVENT_TYPE_UNKNOWN;
      return elem;
  }
}

android_log_list_element android_log_read_next(android_log_context ctx) {
  return android_log_read_next_internal(ctx, 0);
}

android_log_list_element android_log_peek_next(android_log_context ctx) {
  return android_log_read_next_internal(ctx, 1);
}
