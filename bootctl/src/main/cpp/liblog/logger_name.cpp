/*
** Copyright 2013-2014, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <string.h>
#include <type_traits>

#include <log/log.h>

/* In the future, we would like to make this list extensible */
static const char* LOG_NAME[LOG_ID_MAX] = {
    /* clang-format off */
  [LOG_ID_MAIN] = "main",
  [LOG_ID_RADIO] = "radio",
  [LOG_ID_EVENTS] = "events",
  [LOG_ID_SYSTEM] = "system",
  [LOG_ID_CRASH] = "crash",
  [LOG_ID_STATS] = "stats",
  [LOG_ID_SECURITY] = "security",
  [LOG_ID_KERNEL] = "kernel",
    /* clang-format on */
};

const char* android_log_id_to_name(log_id_t log_id) {
  if (log_id >= LOG_ID_MAX) {
    log_id = LOG_ID_MAIN;
  }
  return LOG_NAME[log_id];
}

static_assert(std::is_same<std::underlying_type<log_id_t>::type, uint32_t>::value,
              "log_id_t must be an uint32_t");

static_assert(std::is_same<std::underlying_type<android_LogPriority>::type, uint32_t>::value,
              "log_id_t must be an uint32_t");

log_id_t android_name_to_log_id(const char* logName) {
  const char* b;
  unsigned int ret;

  if (!logName) {
    return static_cast<log_id_t>(LOG_ID_MAX);
  }

  b = strrchr(logName, '/');
  if (!b) {
    b = logName;
  } else {
    ++b;
  }

  for (ret = LOG_ID_MIN; ret < LOG_ID_MAX; ++ret) {
    const char* l = LOG_NAME[ret];
    if (l && !strcmp(b, l)) {
      return static_cast<log_id_t>(ret);
    }
  }

  return static_cast<log_id_t>(LOG_ID_MAX);
}
