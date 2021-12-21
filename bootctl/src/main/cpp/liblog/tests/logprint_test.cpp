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

#include <log/logprint.h>

#include <string>

#include <gtest/gtest.h>

#include <log/log_read.h>

size_t convertPrintable(char* p, const char* message, size_t messageLen);

TEST(liblog, convertPrintable_ascii) {
  auto input = "easy string, output same";
  auto output_size = convertPrintable(nullptr, input, strlen(input));
  EXPECT_EQ(output_size, strlen(input));

  char output[output_size];

  output_size = convertPrintable(output, input, strlen(input));
  EXPECT_EQ(output_size, strlen(input));
  EXPECT_STREQ(input, output);
}

TEST(liblog, convertPrintable_escapes) {
  // Note that \t is not escaped.
  auto input = "escape\a\b\t\v\f\r\\";
  auto expected_output = "escape\\a\\b\t\\v\\f\\r\\\\";
  auto output_size = convertPrintable(nullptr, input, strlen(input));
  EXPECT_EQ(output_size, strlen(expected_output));

  char output[output_size];

  output_size = convertPrintable(output, input, strlen(input));
  EXPECT_EQ(output_size, strlen(expected_output));
  EXPECT_STREQ(expected_output, output);
}

TEST(liblog, convertPrintable_validutf8) {
  auto input = u8"¢ह€𐍈";
  auto output_size = convertPrintable(nullptr, input, strlen(input));
  EXPECT_EQ(output_size, strlen(input));

  char output[output_size];

  output_size = convertPrintable(output, input, strlen(input));
  EXPECT_EQ(output_size, strlen(input));
  EXPECT_STREQ(input, output);
}

TEST(liblog, convertPrintable_invalidutf8) {
  auto input = "\x80\xC2\x01\xE0\xA4\x06\xE0\x06\xF0\x90\x8D\x06\xF0\x90\x06\xF0\x0E";
  auto expected_output =
      "\\x80\\xC2\\x01\\xE0\\xA4\\x06\\xE0\\x06\\xF0\\x90\\x8D\\x06\\xF0\\x90\\x06\\xF0\\x0E";
  auto output_size = convertPrintable(nullptr, input, strlen(input));
  EXPECT_EQ(output_size, strlen(expected_output));

  char output[output_size];

  output_size = convertPrintable(output, input, strlen(input));
  EXPECT_EQ(output_size, strlen(expected_output));
  EXPECT_STREQ(expected_output, output);
}

TEST(liblog, convertPrintable_mixed) {
  auto input =
      u8"\x80\xC2¢ह€𐍈\x01\xE0\xA4\x06¢ह€𐍈\xE0\x06\a\b\xF0\x90¢ह€𐍈\x8D\x06\xF0\t\t\x90\x06\xF0\x0E";
  auto expected_output =
      u8"\\x80\\xC2¢ह€𐍈\\x01\\xE0\\xA4\\x06¢ह€𐍈\\xE0\\x06\\a\\b\\xF0\\x90¢ह€𐍈\\x8D\\x06\\xF0\t\t"
      u8"\\x90\\x06\\xF0\\x0E";
  auto output_size = convertPrintable(nullptr, input, strlen(input));
  EXPECT_EQ(output_size, strlen(expected_output));

  char output[output_size];

  output_size = convertPrintable(output, input, strlen(input));
  EXPECT_EQ(output_size, strlen(expected_output));
  EXPECT_STREQ(expected_output, output);
}

TEST(liblog, log_print_different_header_size) {
  constexpr int32_t kPid = 123;
  constexpr uint32_t kTid = 456;
  constexpr uint32_t kSec = 1000;
  constexpr uint32_t kNsec = 999;
  constexpr uint32_t kLid = LOG_ID_MAIN;
  constexpr uint32_t kUid = 987;
  constexpr char kPriority = ANDROID_LOG_ERROR;

  auto create_buf = [](char* buf, size_t len, uint16_t hdr_size) {
    memset(buf, 0, len);
    logger_entry* header = reinterpret_cast<logger_entry*>(buf);
    header->hdr_size = hdr_size;
    header->pid = kPid;
    header->tid = kTid;
    header->sec = kSec;
    header->nsec = kNsec;
    header->lid = kLid;
    header->uid = kUid;
    char* message = buf + header->hdr_size;
    uint16_t message_len = 0;
    message[message_len++] = kPriority;
    message[message_len++] = 'T';
    message[message_len++] = 'a';
    message[message_len++] = 'g';
    message[message_len++] = '\0';
    message[message_len++] = 'm';
    message[message_len++] = 's';
    message[message_len++] = 'g';
    message[message_len++] = '!';
    message[message_len++] = '\0';
    header->len = message_len;
  };

  auto check_entry = [&](const AndroidLogEntry& entry) {
    EXPECT_EQ(kSec, static_cast<uint32_t>(entry.tv_sec));
    EXPECT_EQ(kNsec, static_cast<uint32_t>(entry.tv_nsec));
    EXPECT_EQ(kPriority, entry.priority);
    EXPECT_EQ(kUid, static_cast<uint32_t>(entry.uid));
    EXPECT_EQ(kPid, entry.pid);
    EXPECT_EQ(kTid, static_cast<uint32_t>(entry.tid));
    EXPECT_STREQ("Tag", entry.tag);
    EXPECT_EQ(4U, entry.tagLen);  // Apparently taglen includes the nullptr?
    EXPECT_EQ(4U, entry.messageLen);
    EXPECT_STREQ("msg!", entry.message);
  };
  alignas(logger_entry) char buf[LOGGER_ENTRY_MAX_LEN];
  create_buf(buf, sizeof(buf), sizeof(logger_entry));

  AndroidLogEntry entry_normal_size;
  ASSERT_EQ(0,
            android_log_processLogBuffer(reinterpret_cast<logger_entry*>(buf), &entry_normal_size));
  check_entry(entry_normal_size);

  create_buf(buf, sizeof(buf), sizeof(logger_entry) + 3);
  AndroidLogEntry entry_odd_size;
  ASSERT_EQ(0, android_log_processLogBuffer(reinterpret_cast<logger_entry*>(buf), &entry_odd_size));
  check_entry(entry_odd_size);
}