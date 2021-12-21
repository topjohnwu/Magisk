/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <log/log.h>
#include <private/android_logger.h>

#include <stdlib.h>
#include <unistd.h>

#include <regex>
#include <string>

#include <android-base/logging.h>
#include <android-base/macros.h>
#include <android-base/stringprintf.h>
#include <android-base/test_utils.h>
#include <gtest/gtest.h>

using android::base::InitLogging;
using android::base::StderrLogger;
using android::base::StringPrintf;

static std::string MakeLogPattern(int priority, const char* tag, const char* message) {
  static const char log_characters[] = "XXVDIWEF";
  static_assert(arraysize(log_characters) - 1 == ANDROID_LOG_SILENT,
                "Mismatch in size of log_characters and values in android_LogPriority");
  priority = priority > ANDROID_LOG_SILENT ? ANDROID_LOG_FATAL : priority;
  char log_char = log_characters[priority];

  return StringPrintf("%s %c \\d+-\\d+ \\d+:\\d+:\\d+ \\s*\\d+ \\s*\\d+ %s", tag, log_char,
                      message);
}

static void CheckMessage(bool expected, const std::string& output, int priority, const char* tag,
                         const char* message) {
  std::regex message_regex(MakeLogPattern(priority, tag, message));
  EXPECT_EQ(expected, std::regex_search(output, message_regex)) << message;
}

static void GenerateLogContent() {
  __android_log_buf_print(LOG_ID_MAIN, ANDROID_LOG_VERBOSE, "tag", "verbose main");
  __android_log_buf_print(LOG_ID_MAIN, ANDROID_LOG_INFO, "tag", "info main");
  __android_log_buf_print(LOG_ID_MAIN, ANDROID_LOG_ERROR, "tag", "error main");

  __android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_VERBOSE, "tag", "verbose radio");
  __android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_INFO, "tag", "info radio");
  __android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_ERROR, "tag", "error radio");

  __android_log_buf_print(LOG_ID_SYSTEM, ANDROID_LOG_VERBOSE, "tag", "verbose system");
  __android_log_buf_print(LOG_ID_SYSTEM, ANDROID_LOG_INFO, "tag", "info system");
  __android_log_buf_print(LOG_ID_SYSTEM, ANDROID_LOG_ERROR, "tag", "error system");

  __android_log_buf_print(LOG_ID_CRASH, ANDROID_LOG_VERBOSE, "tag", "verbose crash");
  __android_log_buf_print(LOG_ID_CRASH, ANDROID_LOG_INFO, "tag", "info crash");
  __android_log_buf_print(LOG_ID_CRASH, ANDROID_LOG_ERROR, "tag", "error crash");
}

std::string GetPidString() {
  int pid = getpid();
  return StringPrintf("%5d", pid);
}

TEST(liblog, default_write) {
  CapturedStderr captured_stderr;
  InitLogging(nullptr, StderrLogger);

  GenerateLogContent();

  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose main");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info main");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error main");

  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose radio");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info radio");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error radio");

  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose system");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info system");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error system");

  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose crash");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info crash");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error crash");
}

TEST(liblog, verbose_write) {
  setenv("ANDROID_LOG_TAGS", "*:v", true);
  CapturedStderr captured_stderr;
  InitLogging(nullptr, StderrLogger);

  GenerateLogContent();

  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose main");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info main");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error main");

  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose radio");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info radio");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error radio");

  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose system");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info system");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error system");

  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose crash");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info crash");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error crash");
}

TEST(liblog, error_write) {
  setenv("ANDROID_LOG_TAGS", "*:e", true);
  CapturedStderr captured_stderr;
  InitLogging(nullptr, StderrLogger);

  GenerateLogContent();

  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose main");
  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info main");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error main");

  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose radio");
  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info radio");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error radio");

  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose system");
  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info system");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error system");

  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_VERBOSE, "tag", "verbose crash");
  CheckMessage(false, captured_stderr.str(), ANDROID_LOG_INFO, "tag", "info crash");
  CheckMessage(true, captured_stderr.str(), ANDROID_LOG_ERROR, "tag", "error crash");
}

TEST(liblog, kernel_no_write) {
  CapturedStderr captured_stderr;
  InitLogging(nullptr, StderrLogger);
  __android_log_buf_print(LOG_ID_KERNEL, ANDROID_LOG_ERROR, "tag", "kernel error");
  EXPECT_EQ("", captured_stderr.str());
}

TEST(liblog, binary_no_write) {
  CapturedStderr captured_stderr;
  InitLogging(nullptr, StderrLogger);
  __android_log_buf_print(LOG_ID_EVENTS, ANDROID_LOG_ERROR, "tag", "error events");
  __android_log_buf_print(LOG_ID_STATS, ANDROID_LOG_ERROR, "tag", "error stats");
  __android_log_buf_print(LOG_ID_SECURITY, ANDROID_LOG_ERROR, "tag", "error security");

  __android_log_bswrite(0x12, "events");
  __android_log_stats_bwrite(0x34, "stats", strlen("stats"));
  __android_log_security_bswrite(0x56, "security");

  EXPECT_EQ("", captured_stderr.str());
}
