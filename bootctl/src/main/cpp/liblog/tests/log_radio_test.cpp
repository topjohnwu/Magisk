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
#include <log/log_radio.h>

TEST(liblog, RLOG) {
  static const char content[] = "log_radio.h";
  static const char content_false[] = "log_radio.h false";

// ratelimit content to 10/s to keep away from spam filters
// do not send identical content together to keep away from spam filters

#undef LOG_TAG
#define LOG_TAG "TEST__RLOGV"
  RLOGV(content);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__RLOGD"
  RLOGD(content);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__RLOGI"
  RLOGI(content);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__RLOGW"
  RLOGW(content);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__RLOGE"
  RLOGE(content);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__RLOGV"
  RLOGV_IF(true, content);
  usleep(100000);
  RLOGV_IF(false, content_false);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__RLOGD"
  RLOGD_IF(true, content);
  usleep(100000);
  RLOGD_IF(false, content_false);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__RLOGI"
  RLOGI_IF(true, content);
  usleep(100000);
  RLOGI_IF(false, content_false);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__RLOGW"
  RLOGW_IF(true, content);
  usleep(100000);
  RLOGW_IF(false, content_false);
  usleep(100000);
#undef LOG_TAG
#define LOG_TAG "TEST__RLOGE"
  RLOGE_IF(true, content);
  usleep(100000);
  RLOGE_IF(false, content_false);

#ifdef __ANDROID__
  // give time for content to long-path through logger
  sleep(1);

  std::string buf = android::base::StringPrintf(
      "logcat -b radio --pid=%u -d -s"
      " TEST__RLOGV TEST__RLOGD TEST__RLOGI TEST__RLOGW TEST__RLOGE",
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
