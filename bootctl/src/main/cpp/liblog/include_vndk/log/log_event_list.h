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

/* Special log_event_list.h file for VNDK linking modules */

#ifndef _LIBS_LOG_EVENT_LIST_H
#define _LIBS_LOG_EVENT_LIST_H

#include <stdint.h>

#include <log/log_id.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The opaque context used to manipulate lists of events.
 */
#ifndef __android_log_context_defined
#define __android_log_context_defined
typedef struct android_log_context_internal* android_log_context;
#endif

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

/* Reset writer context */
int android_log_reset(android_log_context ctx);

/* Reset reader context */
int android_log_parser_reset(android_log_context ctx,
                             const char* msg, size_t len);

/* Finished with reader or writer context */
int android_log_destroy(android_log_context* ctx);

#ifdef __cplusplus
}
#endif

#endif /* _LIBS_LOG_EVENT_LIST_H */
