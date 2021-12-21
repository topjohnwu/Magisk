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

#include "logging_splitters.h"

#include <string>
#include <vector>

#include <android-base/strings.h>
#include <gtest/gtest.h>

namespace android {
namespace base {

void TestNewlineSplitter(const std::string& input,
                         const std::vector<std::string>& expected_output) {
  std::vector<std::string> output;
  auto logger_function = [&](const char* msg, int length) {
    if (length == -1) {
      output.push_back(msg);
    } else {
      output.push_back(std::string(msg, length));
    }
  };
  SplitByLines(input.c_str(), logger_function);

  EXPECT_EQ(expected_output, output);
}

TEST(logging_splitters, NewlineSplitter_EmptyString) {
  TestNewlineSplitter("", std::vector<std::string>{""});
}

TEST(logging_splitters, NewlineSplitter_BasicString) {
  TestNewlineSplitter("normal string", std::vector<std::string>{"normal string"});
}

TEST(logging_splitters, NewlineSplitter_ormalBasicStringTrailingNewline) {
  TestNewlineSplitter("normal string\n", std::vector<std::string>{"normal string", ""});
}

TEST(logging_splitters, NewlineSplitter_MultilineTrailing) {
  TestNewlineSplitter("normal string\nsecond string\nthirdstring",
                      std::vector<std::string>{"normal string", "second string", "thirdstring"});
}

TEST(logging_splitters, NewlineSplitter_MultilineTrailingNewline) {
  TestNewlineSplitter(
      "normal string\nsecond string\nthirdstring\n",
      std::vector<std::string>{"normal string", "second string", "thirdstring", ""});
}

TEST(logging_splitters, NewlineSplitter_MultilineEmbeddedNewlines) {
  TestNewlineSplitter(
      "normal string\n\n\nsecond string\n\nthirdstring\n",
      std::vector<std::string>{"normal string", "", "", "second string", "", "thirdstring", ""});
}

void TestLogdChunkSplitter(const std::string& tag, const std::string& file,
                           const std::string& input,
                           const std::vector<std::string>& expected_output) {
  std::vector<std::string> output;
  auto logger_function = [&](LogId, LogSeverity, const char*, const char* msg) {
    output.push_back(msg);
  };

  SplitByLogdChunks(MAIN, FATAL, tag.c_str(), file.empty() ? nullptr : file.c_str(), 1000,
                    input.c_str(), logger_function);

  auto return_lengths = [&] {
    std::string sizes;
    sizes += "expected_output sizes:";
    for (const auto& string : expected_output) {
      sizes += " " + std::to_string(string.size());
    }
    sizes += "\noutput sizes:";
    for (const auto& string : output) {
      sizes += " " + std::to_string(string.size());
    }
    return sizes;
  };

  EXPECT_EQ(expected_output, output) << return_lengths();
}

TEST(logging_splitters, LogdChunkSplitter_EmptyString) {
  TestLogdChunkSplitter("tag", "", "", std::vector<std::string>{""});
}

TEST(logging_splitters, LogdChunkSplitter_BasicString) {
  TestLogdChunkSplitter("tag", "", "normal string", std::vector<std::string>{"normal string"});
}

TEST(logging_splitters, LogdChunkSplitter_NormalBasicStringTrailingNewline) {
  TestLogdChunkSplitter("tag", "", "normal string\n", std::vector<std::string>{"normal string\n"});
}

TEST(logging_splitters, LogdChunkSplitter_MultilineTrailing) {
  TestLogdChunkSplitter("tag", "", "normal string\nsecond string\nthirdstring",
                        std::vector<std::string>{"normal string\nsecond string\nthirdstring"});
}

TEST(logging_splitters, LogdChunkSplitter_MultilineTrailingNewline) {
  TestLogdChunkSplitter("tag", "", "normal string\nsecond string\nthirdstring\n",
                        std::vector<std::string>{"normal string\nsecond string\nthirdstring\n"});
}

TEST(logging_splitters, LogdChunkSplitter_MultilineEmbeddedNewlines) {
  TestLogdChunkSplitter(
      "tag", "", "normal string\n\n\nsecond string\n\nthirdstring\n",
      std::vector<std::string>{"normal string\n\n\nsecond string\n\nthirdstring\n"});
}

// This test should return the same string, the logd logger itself will truncate down to size.
// This has historically been the behavior both in libbase and liblog.
TEST(logging_splitters, LogdChunkSplitter_HugeLineNoNewline) {
  auto long_string = std::string(LOGGER_ENTRY_MAX_PAYLOAD, 'x');
  ASSERT_EQ(LOGGER_ENTRY_MAX_PAYLOAD, static_cast<int>(long_string.size()));

  TestLogdChunkSplitter("tag", "", long_string, std::vector{long_string});
}

std::string ReduceToMaxSize(const std::string& tag, const std::string& string) {
  return string.substr(0, LOGGER_ENTRY_MAX_PAYLOAD - tag.size() - 35);
}

TEST(logging_splitters, LogdChunkSplitter_MultipleHugeLineNoNewline) {
  auto long_string_x = std::string(LOGGER_ENTRY_MAX_PAYLOAD, 'x');
  auto long_string_y = std::string(LOGGER_ENTRY_MAX_PAYLOAD, 'y');
  auto long_string_z = std::string(LOGGER_ENTRY_MAX_PAYLOAD, 'z');

  auto long_strings = long_string_x + '\n' + long_string_y + '\n' + long_string_z;

  std::string tag = "tag";
  std::vector expected = {ReduceToMaxSize(tag, long_string_x), ReduceToMaxSize(tag, long_string_y),
                          long_string_z};

  TestLogdChunkSplitter(tag, "", long_strings, expected);
}

// With a ~4k buffer, we should print 2 long strings per logger call.
TEST(logging_splitters, LogdChunkSplitter_Multiple2kLines) {
  std::vector expected = {
      std::string(2000, 'a') + '\n' + std::string(2000, 'b'),
      std::string(2000, 'c') + '\n' + std::string(2000, 'd'),
      std::string(2000, 'e') + '\n' + std::string(2000, 'f'),
  };

  auto long_strings = Join(expected, '\n');

  TestLogdChunkSplitter("tag", "", long_strings, expected);
}

TEST(logging_splitters, LogdChunkSplitter_ExactSizedLines) {
  const char* tag = "tag";
  ptrdiff_t max_size = LOGGER_ENTRY_MAX_PAYLOAD - strlen(tag) - 35;
  auto long_string_a = std::string(max_size, 'a');
  auto long_string_b = std::string(max_size, 'b');
  auto long_string_c = std::string(max_size, 'c');

  auto long_strings = long_string_a + '\n' + long_string_b + '\n' + long_string_c;

  TestLogdChunkSplitter(tag, "", long_strings,
                        std::vector{long_string_a, long_string_b, long_string_c});
}

TEST(logging_splitters, LogdChunkSplitter_UnderEqualOver) {
  std::string tag = "tag";
  ptrdiff_t max_size = LOGGER_ENTRY_MAX_PAYLOAD - tag.size() - 35;

  auto first_string_size = 1000;
  auto first_string = std::string(first_string_size, 'a');
  auto second_string_size = max_size - first_string_size - 1;
  auto second_string = std::string(second_string_size, 'b');

  auto exact_string = std::string(max_size, 'c');

  auto large_string = std::string(max_size + 50, 'd');

  auto final_string = std::string("final string!\n\nfinal \n \n final \n");

  std::vector expected = {first_string + '\n' + second_string, exact_string,
                          ReduceToMaxSize(tag, large_string), final_string};

  std::vector input_strings = {first_string + '\n' + second_string, exact_string, large_string,
                               final_string};
  auto long_strings = Join(input_strings, '\n');

  TestLogdChunkSplitter(tag, "", long_strings, expected);
}

TEST(logging_splitters, LogdChunkSplitter_WithFile) {
  std::string tag = "tag";
  std::string file = "/path/to/myfile.cpp";
  int line = 1000;
  auto file_header = StringPrintf("%s:%d] ", file.c_str(), line);
  ptrdiff_t max_size = LOGGER_ENTRY_MAX_PAYLOAD - tag.size() - 35;

  auto first_string_size = 1000;
  auto first_string = std::string(first_string_size, 'a');
  auto second_string_size = max_size - first_string_size - 1 - 2 * file_header.size();
  auto second_string = std::string(second_string_size, 'b');

  auto exact_string = std::string(max_size - file_header.size(), 'c');

  auto large_string = std::string(max_size + 50, 'd');

  auto final_string = std::string("final string!");

  std::vector expected = {
      file_header + first_string + '\n' + file_header + second_string, file_header + exact_string,
      file_header + ReduceToMaxSize(file_header + tag, large_string), file_header + final_string};

  std::vector input_strings = {first_string + '\n' + second_string, exact_string, large_string,
                               final_string};
  auto long_strings = Join(input_strings, '\n');

  TestLogdChunkSplitter(tag, file, long_strings, expected);
}

// We set max_size based off of tag, so if it's too large, the buffer will be sized wrong.
// We could recover from this, but it's certainly an error for someone to attempt to use a tag this
// large, so we abort instead.
TEST(logging_splitters, LogdChunkSplitter_TooLongTag) {
  auto long_tag = std::string(5000, 'x');
  auto logger_function = [](LogId, LogSeverity, const char*, const char*) {};
  ASSERT_DEATH(
      SplitByLogdChunks(MAIN, ERROR, long_tag.c_str(), nullptr, 0, "message", logger_function), "");
}

// We do handle excessively large file names correctly however.
TEST(logging_splitters, LogdChunkSplitter_TooLongFile) {
  auto long_file = std::string(5000, 'x');
  std::string tag = "tag";

  std::vector expected = {ReduceToMaxSize(tag, long_file), ReduceToMaxSize(tag, long_file)};

  TestLogdChunkSplitter(tag, long_file, "can't see me\nor me", expected);
}

void TestStderrOutputGenerator(const char* tag, const char* file, int line, const char* message,
                               const std::string& expected) {
  // All log messages will show "01-01 00:00:00"
  struct tm now = {
      .tm_sec = 0,
      .tm_min = 0,
      .tm_hour = 0,
      .tm_mday = 1,
      .tm_mon = 0,
      .tm_year = 1970,
  };

  int pid = 1234;       // All log messages will have 1234 for their PID.
  uint64_t tid = 4321;  // All log messages will have 4321 for their TID.

  auto result = StderrOutputGenerator(now, pid, tid, ERROR, tag, file, line, message);
  EXPECT_EQ(expected, result);
}

TEST(logging_splitters, StderrOutputGenerator_Basic) {
  TestStderrOutputGenerator(nullptr, nullptr, 0, "simple message",
                            "nullptr E 01-01 00:00:00  1234  4321 simple message\n");
  TestStderrOutputGenerator("tag", nullptr, 0, "simple message",
                            "tag E 01-01 00:00:00  1234  4321 simple message\n");
  TestStderrOutputGenerator(
      "tag", "/path/to/some/file", 0, "simple message",
      "tag E 01-01 00:00:00  1234  4321 /path/to/some/file:0] simple message\n");
}

TEST(logging_splitters, StderrOutputGenerator_NewlineTagAndFile) {
  TestStderrOutputGenerator("tag\n\n", nullptr, 0, "simple message",
                            "tag\n\n E 01-01 00:00:00  1234  4321 simple message\n");
  TestStderrOutputGenerator(
      "tag", "/path/to/some/file\n\n", 0, "simple message",
      "tag E 01-01 00:00:00  1234  4321 /path/to/some/file\n\n:0] simple message\n");
}

TEST(logging_splitters, StderrOutputGenerator_TrailingNewLine) {
  TestStderrOutputGenerator(
      "tag", nullptr, 0, "simple message\n",
      "tag E 01-01 00:00:00  1234  4321 simple message\ntag E 01-01 00:00:00  1234  4321 \n");
}

TEST(logging_splitters, StderrOutputGenerator_MultiLine) {
  const char* expected_result =
      "tag E 01-01 00:00:00  1234  4321 simple message\n"
      "tag E 01-01 00:00:00  1234  4321 \n"
      "tag E 01-01 00:00:00  1234  4321 \n"
      "tag E 01-01 00:00:00  1234  4321 another message \n"
      "tag E 01-01 00:00:00  1234  4321 \n"
      "tag E 01-01 00:00:00  1234  4321  final message \n"
      "tag E 01-01 00:00:00  1234  4321 \n"
      "tag E 01-01 00:00:00  1234  4321 \n"
      "tag E 01-01 00:00:00  1234  4321 \n";

  TestStderrOutputGenerator("tag", nullptr, 0,
                            "simple message\n\n\nanother message \n\n final message \n\n\n",
                            expected_result);
}

TEST(logging_splitters, StderrOutputGenerator_MultiLineLong) {
  auto long_string_a = std::string(4000, 'a');
  auto long_string_b = std::string(4000, 'b');

  auto message = long_string_a + '\n' + long_string_b;
  auto expected_result = "tag E 01-01 00:00:00  1234  4321 " + long_string_a + '\n' +
                         "tag E 01-01 00:00:00  1234  4321 " + long_string_b + '\n';
  TestStderrOutputGenerator("tag", nullptr, 0, message.c_str(), expected_result);
}

}  // namespace base
}  // namespace android
