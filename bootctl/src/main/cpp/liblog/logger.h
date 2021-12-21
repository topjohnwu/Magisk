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

#pragma once

#include <stdatomic.h>
#include <sys/cdefs.h>

#include <log/log.h>

#include "uio.h"

__BEGIN_DECLS

struct logger_list {
  atomic_int fd;
  int mode;
  unsigned int tail;
  log_time start;
  pid_t pid;
  uint32_t log_mask;
};

// Format for a 'logger' entry: uintptr_t where only the bottom 32 bits are used.
// bit 31: Set if this 'logger' is for logd.
// bit 30: Set if this 'logger' is for pmsg
// bits 0-2: the decimal value of the log buffer.
// Other bits are unused.

#define LOGGER_LOGD (1U << 31)
#define LOGGER_PMSG (1U << 30)
#define LOGGER_LOG_ID_MASK ((1U << 3) - 1)

inline bool android_logger_is_logd(struct logger* logger) {
  return reinterpret_cast<uintptr_t>(logger) & LOGGER_LOGD;
}

__END_DECLS
