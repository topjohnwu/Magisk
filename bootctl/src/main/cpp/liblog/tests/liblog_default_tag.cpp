/*
 * Copyright (C) 2020 The Android Open Source Project
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

// LOG_TAG must be unset for android-base's logging to use a default tag.
#undef LOG_TAG

#include <stdlib.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/scopeguard.h>
#include <android/log.h>

#include <gtest/gtest.h>

#ifndef __ANDROID__
static const char* getprogname() {
  return program_invocation_short_name;
}
#endif

TEST(liblog_default_tag, no_default_tag_libbase_write_first) {
  using namespace android::base;
  bool message_seen = false;
  std::string expected_tag = "";
  SetLogger([&](LogId, LogSeverity, const char* tag, const char*, unsigned int, const char*) {
    message_seen = true;
    EXPECT_EQ(expected_tag, tag);
  });

  expected_tag = getprogname();
  LOG(WARNING) << "message";
  EXPECT_TRUE(message_seen);
  message_seen = false;

  __android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_WARN, nullptr, "message");
  EXPECT_TRUE(message_seen);
}

TEST(liblog_default_tag, no_default_tag_liblog_write_first) {
  using namespace android::base;
  bool message_seen = false;
  std::string expected_tag = "";
  SetLogger([&](LogId, LogSeverity, const char* tag, const char*, unsigned int, const char*) {
    message_seen = true;
    EXPECT_EQ(expected_tag, tag);
  });

  expected_tag = getprogname();
  __android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_WARN, nullptr, "message");
  EXPECT_TRUE(message_seen);
  message_seen = false;

  LOG(WARNING) << "message";
  EXPECT_TRUE(message_seen);
}

TEST(liblog_default_tag, libbase_sets_default_tag) {
  using namespace android::base;
  bool message_seen = false;
  std::string expected_tag = "libbase_test_tag";
  SetLogger([&](LogId, LogSeverity, const char* tag, const char*, unsigned int, const char*) {
    message_seen = true;
    EXPECT_EQ(expected_tag, tag);
  });
  SetDefaultTag(expected_tag);

  __android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_WARN, nullptr, "message");
  EXPECT_TRUE(message_seen);
  message_seen = false;

  LOG(WARNING) << "message";
  EXPECT_TRUE(message_seen);
}

TEST(liblog_default_tag, liblog_sets_default_tag) {
  using namespace android::base;
  bool message_seen = false;
  std::string expected_tag = "liblog_test_tag";
  SetLogger([&](LogId, LogSeverity, const char* tag, const char*, unsigned int, const char*) {
    message_seen = true;
    EXPECT_EQ(expected_tag, tag);
  });
  __android_log_set_default_tag(expected_tag.c_str());

  __android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_WARN, nullptr, "message");
  EXPECT_TRUE(message_seen);
  message_seen = false;

  LOG(WARNING) << "message";
  EXPECT_TRUE(message_seen);
}

TEST(liblog_default_tag, default_tag_plus_log_severity) {
#ifdef __ANDROID__
  using namespace android::base;
  bool message_seen = false;
  std::string expected_tag = "liblog_test_tag";
  SetLogger([&](LogId, LogSeverity, const char* tag, const char*, unsigned int, const char*) {
    message_seen = true;
    EXPECT_EQ(expected_tag, tag);
  });
  __android_log_set_default_tag(expected_tag.c_str());

  auto log_tag_property = "log.tag." + expected_tag;
  SetProperty(log_tag_property, "V");
  auto reset_tag_property_guard = make_scope_guard([=] { SetProperty(log_tag_property, ""); });

  __android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_VERBOSE, nullptr, "message");
  EXPECT_TRUE(message_seen);
  message_seen = false;

  LOG(VERBOSE) << "message";
  EXPECT_TRUE(message_seen);
#else
  GTEST_SKIP() << "No log tag properties on host";
#endif
}

TEST(liblog_default_tag, generated_default_tag_plus_log_severity) {
#ifdef __ANDROID__
  using namespace android::base;
  bool message_seen = false;
  std::string expected_tag = getprogname();
  SetLogger([&](LogId, LogSeverity, const char* tag, const char*, unsigned int, const char*) {
    message_seen = true;
    EXPECT_EQ(expected_tag, tag);
  });

  // Even without any calls to SetDefaultTag(), the first message that attempts to log, will
  // generate a default tag from getprogname() and check log.tag.<default tag> for loggability. This
  // case checks that we can log a Verbose message when log.tag.<getprogname()> is set to 'V'.
  auto log_tag_property = "log.tag." + expected_tag;
  SetProperty(log_tag_property, "V");
  auto reset_tag_property_guard = make_scope_guard([=] { SetProperty(log_tag_property, ""); });

  __android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_VERBOSE, nullptr, "message");
  EXPECT_TRUE(message_seen);
  message_seen = false;

  LOG(VERBOSE) << "message";
  EXPECT_TRUE(message_seen);
#else
  GTEST_SKIP() << "No log tag properties on host";
#endif
}