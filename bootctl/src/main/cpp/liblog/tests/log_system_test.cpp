/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <gtest/gtest.h>
// Test the APIs in this standalone include file
#include <log/log_system.h>

TEST(liblog, SLOG) {
  static const char content[] = "log_system.h";
  static const char content_false[] = "log_system.h false";

// ratelimit content to 10/s to keep away from spam filters
// do not send identical content together to keep away from spam filters

#undef LOG_TAG
#define LOG_TAG "TEST__SLOGV"
  SLOGV(content);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__SLOGD"
  SLOGD(content);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__SLOGI"
  SLOGI(content);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__SLOGW"
  SLOGW(content);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__SLOGE"
  SLOGE(content);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__SLOGV"
  SLOGV_IF(true, content);
  usleep(100000);
  SLOGV_IF(false, content_false);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__SLOGD"
  SLOGD_IF(true, content);
  usleep(100000);
  SLOGD_IF(false, content_false);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__SLOGI"
  SLOGI_IF(true, content);
  usleep(100000);
  SLOGI_IF(false, content_false);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__SLOGW"
  SLOGW_IF(true, content);
  usleep(100000);
  SLOGW_IF(false, content_false);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__SLOGE"
  SLOGE_IF(true, content);
  usleep(100000);
  SLOGE_IF(false, content_false);

#ifdef __ANDROID__
  // give time for content to long-path through logger
  sleep(1);

  std::string buf = android::base::StringPrintf(
      "logcat -b system --pid=%u -d -s"
      " TEST__SLOGV TEST__SLOGD TEST__SLOGI TEST__SLOGW TEST__SLOGE",
      (unsigned)getpid());
  FILE* fp = popen(buf.c_str(), "re");
  int count = 0;
  int count_false = 0;
  if (fp) {
    if (!android::base::ReadFdToString(fileno(fp), &buf)) buf = "";
    pclose(fp);
    for (size_t pos = 0; (pos = buf.find(content, pos)) != std::string::npos;
         ++pos) {
      ++count;
    }
    for (size_t pos = 0;
         (pos = buf.find(content_false, pos)) != std::string::npos; ++pos) {
      ++count_false;
    }
  }
  EXPECT_EQ(0, count_false);
#if LOG_NDEBUG
  ASSERT_EQ(8, count);
#else
  ASSERT_EQ(10, count);
#endif

#else
  GTEST_LOG_(INFO) << "This test does not test end-to-end.\n";
#endif
}
