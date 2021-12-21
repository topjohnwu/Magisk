/*
 * Copyright (C) 2013-2017 The Android Open Source Project
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

#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <string>

#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android/log.h>  // minimal logging API
#include <gtest/gtest.h>
#include <log/log_properties.h>
// Test the APIs in this standalone include file
#include <log/log_read.h>
// Do not use anything in log/log_time.h despite side effects of the above.
#include <private/android_logger.h>

using android::base::GetBoolProperty;

TEST(liblog, android_logger_get_) {
#ifdef __ANDROID__
  // This test assumes the log buffers are filled with noise from
  // normal operations. It will fail if done immediately after a
  // logcat -c.
  struct logger_list* logger_list = android_logger_list_alloc(0, 0, 0);

  for (int i = LOG_ID_MIN; i < LOG_ID_MAX; ++i) {
    log_id_t id = static_cast<log_id_t>(i);
    std::string name = android_log_id_to_name(id);
    fprintf(stderr, "log buffer %s\r", name.c_str());
    struct logger* logger;
    EXPECT_TRUE(NULL != (logger = android_logger_open(logger_list, id)));
    EXPECT_EQ(id, android_logger_get_id(logger));
    ssize_t get_log_size = android_logger_get_log_size(logger);
    /* security buffer is allowed to be denied */
    if (name != "security") {
      EXPECT_GT(get_log_size, 0);
      // crash buffer is allowed to be empty, that is actually healthy!
      // stats buffer is no longer in use.
      if (name == "crash" || name == "stats") {
        continue;
      }

      // kernel buffer is empty if ro.logd.kernel is false
      if (name == "kernel" && !GetBoolProperty("ro.logd.kernel", false)) {
        continue;
      }

      EXPECT_LE(0, android_logger_get_log_readable_size(logger));
    } else {
      EXPECT_NE(0, get_log_size);
      if (get_log_size < 0) {
        EXPECT_GT(0, android_logger_get_log_readable_size(logger));
      } else {
        EXPECT_LE(0, android_logger_get_log_readable_size(logger));
      }
    }
  }

  android_logger_list_close(logger_list);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}
