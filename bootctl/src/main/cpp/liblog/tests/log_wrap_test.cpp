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

#include <android-base/chrono_utils.h>
#include <android-base/stringprintf.h>
#include <android/log.h>  // minimal logging API
#include <gtest/gtest.h>
#include <log/log_properties.h>
#include <log/log_read.h>
#include <log/log_time.h>

#ifdef __ANDROID__
static void read_with_wrap() {
  // Read the last line in the log to get a starting timestamp. We're assuming
  // the log is not empty.
  const int mode = ANDROID_LOG_NONBLOCK;
  struct logger_list* logger_list =
      android_logger_list_open(LOG_ID_MAIN, mode, 1000, 0);

  ASSERT_NE(logger_list, nullptr);

  log_msg log_msg;
  int ret = android_logger_list_read(logger_list, &log_msg);
  android_logger_list_close(logger_list);
  ASSERT_GT(ret, 0);

  log_time start(log_msg.entry.sec, log_msg.entry.nsec);
  ASSERT_NE(start, log_time());

  logger_list =
      android_logger_list_alloc_time(mode | ANDROID_LOG_WRAP, start, 0);
  ASSERT_NE(logger_list, nullptr);

  struct logger* logger = android_logger_open(logger_list, LOG_ID_MAIN);
  EXPECT_NE(logger, nullptr);
  if (logger) {
    android_logger_list_read(logger_list, &log_msg);
  }

  android_logger_list_close(logger_list);
}
#endif

// b/64143705 confirm fixed
TEST(liblog, wrap_mode_blocks) {
#ifdef __ANDROID__
  // The read call is expected to take up to 2 hours in the happy case.  There was a previous bug
  // where it would take only 30 seconds due to an alarm() in logd_reader.cpp.  That alarm has been
  // removed, so we check here that the read call blocks for a reasonable amount of time (5s).

  struct sigaction ignore = {.sa_handler = [](int) { _exit(0); }};
  struct sigaction old_sigaction;
  sigaction(SIGALRM, &ignore, &old_sigaction);
  alarm(5);

  android::base::Timer timer;
  read_with_wrap();

  FAIL() << "read_with_wrap() should not return before the alarm is triggered.";

  alarm(0);
  sigaction(SIGALRM, &old_sigaction, nullptr);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}
