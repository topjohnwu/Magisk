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

#include "android-base/test_utils.h"

#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>

namespace android {
namespace base {

TEST(TestUtilsTest, AssertMatch) {
  ASSERT_MATCH("foobar", R"(fo+baz?r)");
  EXPECT_FATAL_FAILURE(ASSERT_MATCH("foobar", R"(foobaz)"), "regex mismatch");
}

TEST(TestUtilsTest, AssertNotMatch) {
  ASSERT_NOT_MATCH("foobar", R"(foobaz)");
  EXPECT_FATAL_FAILURE(ASSERT_NOT_MATCH("foobar", R"(foobar)"), "regex mismatch");
}

TEST(TestUtilsTest, ExpectMatch) {
  EXPECT_MATCH("foobar", R"(fo+baz?r)");
  EXPECT_NONFATAL_FAILURE(EXPECT_MATCH("foobar", R"(foobaz)"), "regex mismatch");
}

TEST(TestUtilsTest, ExpectNotMatch) {
  EXPECT_NOT_MATCH("foobar", R"(foobaz)");
  EXPECT_NONFATAL_FAILURE(EXPECT_NOT_MATCH("foobar", R"(foobar)"), "regex mismatch");
}

TEST(TestUtilsTest, CaptureStdout_smoke) {
  CapturedStdout cap;
  printf("This should be captured.\n");
  fflush(stdout);
  cap.Stop();
  printf("This will not be captured.\n");
  fflush(stdout);
  ASSERT_EQ("This should be captured.\n", cap.str());

  cap.Start();
  printf("And this text should be captured too.\n");
  fflush(stdout);
  cap.Stop();
  ASSERT_EQ("This should be captured.\nAnd this text should be captured too.\n", cap.str());

  printf("Still not going to be captured.\n");
  fflush(stdout);
  cap.Reset();
  cap.Start();
  printf("Only this will be captured.\n");
  fflush(stdout);
  ASSERT_EQ("Only this will be captured.\n", cap.str());
}

TEST(TestUtilsTest, CaptureStderr_smoke) {
  CapturedStderr cap;
  fprintf(stderr, "This should be captured.\n");
  cap.Stop();
  fprintf(stderr, "This will not be captured.\n");
  ASSERT_EQ("This should be captured.\n", cap.str());

  cap.Start();
  fprintf(stderr, "And this text should be captured too.\n");
  cap.Stop();
  ASSERT_EQ("This should be captured.\nAnd this text should be captured too.\n", cap.str());

  fprintf(stderr, "Still not going to be captured.\n");
  cap.Reset();
  cap.Start();
  fprintf(stderr, "Only this will be captured.\n");
  ASSERT_EQ("Only this will be captured.\n", cap.str());
}

}  // namespace base
}  // namespace android
