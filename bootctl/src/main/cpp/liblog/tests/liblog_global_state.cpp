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

#define LOG_TAG "global_state_test_tag"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android/log.h>

#include <gtest/gtest.h>

TEST(liblog_global_state, libbase_logs_with_libbase_SetLogger) {
  using namespace android::base;
  bool message_seen = false;
  LogSeverity expected_severity = WARNING;
  std::string expected_file = Basename(__FILE__);
  unsigned int expected_line;
  std::string expected_message = "libbase test message";

  auto LoggerFunction = [&](LogId log_id, LogSeverity severity, const char* tag, const char* file,
                            unsigned int line, const char* message) {
    message_seen = true;
    EXPECT_EQ(DEFAULT, log_id);
    EXPECT_EQ(expected_severity, severity);
    EXPECT_STREQ(LOG_TAG, tag);
    EXPECT_EQ(expected_file, file);
    EXPECT_EQ(expected_line, line);
    EXPECT_EQ(expected_message, message);
  };

  SetLogger(LoggerFunction);

  expected_line = __LINE__ + 1;
  LOG(expected_severity) << expected_message;
  EXPECT_TRUE(message_seen);
}

TEST(liblog_global_state, libbase_logs_with_liblog_set_logger) {
  using namespace android::base;
  // These must be static since they're used by the liblog logger function, which only accepts
  // lambdas without captures.  The items used by the libbase logger are explicitly not static, to
  // ensure that lambdas with captures do work there.
  static bool message_seen = false;
  static std::string expected_file = Basename(__FILE__);
  static unsigned int expected_line;
  static std::string expected_message = "libbase test message";

  auto liblog_logger_function = [](const struct __android_log_message* log_message) {
    message_seen = true;
    EXPECT_EQ(sizeof(__android_log_message), log_message->struct_size);
    EXPECT_EQ(LOG_ID_DEFAULT, log_message->buffer_id);
    EXPECT_EQ(ANDROID_LOG_WARN, log_message->priority);
    EXPECT_STREQ(LOG_TAG, log_message->tag);
    EXPECT_EQ(expected_file, log_message->file);
    EXPECT_EQ(expected_line, log_message->line);
    EXPECT_EQ(expected_message, log_message->message);
  };

  __android_log_set_logger(liblog_logger_function);

  expected_line = __LINE__ + 1;
  LOG(WARNING) << expected_message;
  EXPECT_TRUE(message_seen);
}

TEST(liblog_global_state, liblog_logs_with_libbase_SetLogger) {
  using namespace android::base;
  bool message_seen = false;
  std::string expected_message = "libbase test message";

  auto LoggerFunction = [&](LogId log_id, LogSeverity severity, const char* tag, const char* file,
                            unsigned int line, const char* message) {
    message_seen = true;
    EXPECT_EQ(MAIN, log_id);
    EXPECT_EQ(WARNING, severity);
    EXPECT_STREQ(LOG_TAG, tag);
    EXPECT_EQ(nullptr, file);
    EXPECT_EQ(0U, line);
    EXPECT_EQ(expected_message, message);
  };

  SetLogger(LoggerFunction);

  __android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_WARN, LOG_TAG, expected_message.c_str());
  EXPECT_TRUE(message_seen);
  message_seen = false;
}

TEST(liblog_global_state, liblog_logs_with_liblog_set_logger) {
  using namespace android::base;
  // These must be static since they're used by the liblog logger function, which only accepts
  // lambdas without captures.  The items used by the libbase logger are explicitly not static, to
  // ensure that lambdas with captures do work there.
  static bool message_seen = false;
  static int expected_buffer_id = LOG_ID_MAIN;
  static int expected_priority = ANDROID_LOG_WARN;
  static std::string expected_message = "libbase test message";

  auto liblog_logger_function = [](const struct __android_log_message* log_message) {
    message_seen = true;
    EXPECT_EQ(sizeof(__android_log_message), log_message->struct_size);
    EXPECT_EQ(expected_buffer_id, log_message->buffer_id);
    EXPECT_EQ(expected_priority, log_message->priority);
    EXPECT_STREQ(LOG_TAG, log_message->tag);
    EXPECT_STREQ(nullptr, log_message->file);
    EXPECT_EQ(0U, log_message->line);
    EXPECT_EQ(expected_message, log_message->message);
  };

  __android_log_set_logger(liblog_logger_function);

  __android_log_buf_write(expected_buffer_id, expected_priority, LOG_TAG, expected_message.c_str());
  EXPECT_TRUE(message_seen);
}

TEST(liblog_global_state, SetAborter_with_liblog) {
  using namespace android::base;

  std::string expected_message = "libbase test message";
  static bool message_seen = false;
  auto aborter_function = [&](const char* message) {
    message_seen = true;
    EXPECT_EQ(expected_message, message);
  };

  SetAborter(aborter_function);
  LOG(FATAL) << expected_message;
  EXPECT_TRUE(message_seen);
  message_seen = false;

  static std::string expected_message_static = "libbase test message";
  auto liblog_aborter_function = [](const char* message) {
    message_seen = true;
    EXPECT_EQ(expected_message_static, message);
  };
  __android_log_set_aborter(liblog_aborter_function);
  LOG(FATAL) << expected_message_static;
  EXPECT_TRUE(message_seen);
  message_seen = false;
}

static std::string UniqueLogTag() {
  std::string tag = LOG_TAG;
  tag += "-" + std::to_string(getpid());
  return tag;
}

TEST(liblog_global_state, is_loggable_both_default) {
  auto tag = UniqueLogTag();
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));
}

TEST(liblog_global_state, is_loggable_minimum_log_priority_only) {
  auto tag = UniqueLogTag();
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  EXPECT_EQ(ANDROID_LOG_DEFAULT, __android_log_set_minimum_priority(ANDROID_LOG_DEBUG));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  EXPECT_EQ(ANDROID_LOG_DEBUG, __android_log_set_minimum_priority(ANDROID_LOG_WARN));
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  EXPECT_EQ(android::base::WARNING, android::base::SetMinimumLogSeverity(android::base::DEBUG));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  EXPECT_EQ(android::base::DEBUG, android::base::SetMinimumLogSeverity(android::base::WARNING));
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));
}

TEST(liblog_global_state, is_loggable_tag_log_priority_only) {
#ifdef __ANDROID__
  auto tag = UniqueLogTag();
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  auto log_tag_property = std::string("log.tag.") + tag;
  ASSERT_TRUE(android::base::SetProperty(log_tag_property, "d"));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  ASSERT_TRUE(android::base::SetProperty(log_tag_property, "w"));
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  ASSERT_TRUE(android::base::SetProperty(log_tag_property, ""));
#else
  GTEST_SKIP() << "No log tag properties on host";
#endif
}

TEST(liblog_global_state, is_loggable_both_set) {
#ifdef __ANDROID__
  auto tag = UniqueLogTag();
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  // When both a tag and a minimum priority are set, we use the lower value of the two.

  // tag = warning, minimum_priority = debug, expect 'debug'
  auto log_tag_property = std::string("log.tag.") + tag;
  ASSERT_TRUE(android::base::SetProperty(log_tag_property, "w"));
  EXPECT_EQ(ANDROID_LOG_DEFAULT, __android_log_set_minimum_priority(ANDROID_LOG_DEBUG));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  // tag = warning, minimum_priority = warning, expect 'warning'
  EXPECT_EQ(ANDROID_LOG_DEBUG, __android_log_set_minimum_priority(ANDROID_LOG_WARN));
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(0, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  // tag = debug, minimum_priority = warning, expect 'debug'
  ASSERT_TRUE(android::base::SetProperty(log_tag_property, "d"));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  // tag = debug, minimum_priority = debug, expect 'debug'
  EXPECT_EQ(ANDROID_LOG_WARN, __android_log_set_minimum_priority(ANDROID_LOG_DEBUG));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_DEBUG, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_INFO, tag.c_str(), ANDROID_LOG_INFO));
  EXPECT_EQ(1, __android_log_is_loggable(ANDROID_LOG_WARN, tag.c_str(), ANDROID_LOG_INFO));

  ASSERT_TRUE(android::base::SetProperty(log_tag_property, ""));
#else
  GTEST_SKIP() << "No log tag properties on host";
#endif
}
