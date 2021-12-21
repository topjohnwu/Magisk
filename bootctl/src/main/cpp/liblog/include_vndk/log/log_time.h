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

#ifndef _LIBS_LOG_LOG_TIME_H
#define _LIBS_LOG_LOG_TIME_H

#include <stdint.h>

/* struct log_time is a wire-format variant of struct timespec */
#ifndef NS_PER_SEC
#define NS_PER_SEC 1000000000ULL
#endif
#ifndef US_PER_SEC
#define US_PER_SEC 1000000ULL
#endif
#ifndef MS_PER_SEC
#define MS_PER_SEC 1000ULL
#endif

#ifndef __struct_log_time_defined
#define __struct_log_time_defined

#define LOG_TIME_SEC(t) ((t)->tv_sec)
/* next power of two after NS_PER_SEC */
#define LOG_TIME_NSEC(t) ((t)->tv_nsec & (UINT32_MAX >> 2))

typedef struct log_time {
  uint32_t tv_sec;
  uint32_t tv_nsec;
} __attribute__((__packed__)) log_time;

#endif

#endif /* _LIBS_LOG_LOG_TIME_H */
