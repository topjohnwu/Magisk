/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "android-base/stringprintf.h"

#include <gtest/gtest.h>

#include <string>

TEST(StringPrintfTest, HexSizeT) {
  size_t size = 0x00107e59;
  EXPECT_EQ("00107e59", android::base::StringPrintf("%08zx", size));
  EXPECT_EQ("0x00107e59", android::base::StringPrintf("0x%08zx", size));
}

TEST(StringPrintfTest, StringAppendF) {
  std::string s("a");
  android::base::StringAppendF(&s, "b");
  EXPECT_EQ("ab", s);
}

TEST(StringPrintfTest, Errno) {
  errno = 123;
  android::base::StringPrintf("hello %s", "world");
  EXPECT_EQ(123, errno);
}

void TestN(size_t n) {
  char* buf = new char[n + 1];
  memset(buf, 'x', n);
  buf[n] = '\0';
  std::string s(android::base::StringPrintf("%s", buf));
  EXPECT_EQ(buf, s);
  delete[] buf;
}

TEST(StringPrintfTest, At1023) {
  TestN(1023);
}

TEST(StringPrintfTest, At1024) {
  TestN(1024);
}

TEST(StringPrintfTest, At1025) {
  TestN(1025);
}
