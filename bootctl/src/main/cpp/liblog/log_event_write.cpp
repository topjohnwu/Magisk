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

#include <errno.h>
#include <stdint.h>

#include <log/log.h>
#include <log/log_event_list.h>

#define MAX_SUBTAG_LEN 32

int __android_log_error_write(int tag, const char* subTag, int32_t uid, const char* data,
                              uint32_t dataLen) {
  int ret = -EINVAL;

  if (subTag && (data || !dataLen)) {
    android_log_context ctx = create_android_logger(tag);

    ret = -ENOMEM;
    if (ctx) {
      ret = android_log_write_string8_len(ctx, subTag, MAX_SUBTAG_LEN);
      if (ret >= 0) {
        ret = android_log_write_int32(ctx, uid);
        if (ret >= 0) {
          ret = android_log_write_string8_len(ctx, data, dataLen);
          if (ret >= 0) {
            ret = android_log_write_list(ctx, LOG_ID_EVENTS);
          }
        }
      }
      android_log_destroy(&ctx);
    }
  }
  return ret;
}
