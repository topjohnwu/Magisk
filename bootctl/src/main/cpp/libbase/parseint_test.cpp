/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "android-base/parseint.h"

#include <errno.h>

#include <gtest/gtest.h>

TEST(parseint, signed_smoke) {
  errno = 0;
  int i = 0;
  ASSERT_FALSE(android::base::ParseInt("x", &i));
  ASSERT_EQ(EINVAL, errno);
  errno = 0;
  ASSERT_FALSE(android::base::ParseInt("123x", &i));
  ASSERT_EQ(EINVAL, errno);

  ASSERT_TRUE(android::base::ParseInt("123", &i));
  ASSERT_EQ(123, i);
  ASSERT_EQ(0, errno);
  i = 0;
  EXPECT_TRUE(android::base::ParseInt("  123", &i));
  EXPECT_EQ(123, i);
  ASSERT_TRUE(android::base::ParseInt("-123", &i));
  ASSERT_EQ(-123, i);
  i = 0;
  EXPECT_TRUE(android::base::ParseInt("  -123", &i));
  EXPECT_EQ(-123, i);

  short s = 0;
  ASSERT_TRUE(android::base::ParseInt("1234", &s));
  ASSERT_EQ(1234, s);

  ASSERT_TRUE(android::base::ParseInt("12", &i, 0, 15));
  ASSERT_EQ(12, i);
  errno = 0;
  ASSERT_FALSE(android::base::ParseInt("-12", &i, 0, 15));
  ASSERT_EQ(ERANGE, errno);
  errno = 0;
  ASSERT_FALSE(android::base::ParseInt("16", &i, 0, 15));
  ASSERT_EQ(ERANGE, errno);

  errno = 0;
  ASSERT_FALSE(android::base::ParseInt<int>("x", nullptr));
  ASSERT_EQ(EINVAL, errno);
  errno = 0;
  ASSERT_FALSE(android::base::ParseInt<int>("123x", nullptr));
  ASSERT_EQ(EINVAL, errno);
  ASSERT_TRUE(android::base::ParseInt<int>("1234", nullptr));
}

TEST(parseint, unsigned_smoke) {
  errno = 0;
  unsigned int i = 0u;
  ASSERT_FALSE(android::base::ParseUint("x", &i));
  ASSERT_EQ(EINVAL, errno);
  errno = 0;
  ASSERT_FALSE(android::base::ParseUint("123x", &i));
  ASSERT_EQ(EINVAL, errno);

  ASSERT_TRUE(android::base::ParseUint("123", &i));
  ASSERT_EQ(123u, i);
  ASSERT_EQ(0, errno);
  i = 0u;
  EXPECT_TRUE(android::base::ParseUint("  123", &i));
  EXPECT_EQ(123u, i);
  errno = 0;
  ASSERT_FALSE(android::base::ParseUint("-123", &i));
  EXPECT_EQ(EINVAL, errno);
  errno = 0;
  EXPECT_FALSE(android::base::ParseUint("  -123", &i));
  EXPECT_EQ(EINVAL, errno);

  unsigned short s = 0u;
  ASSERT_TRUE(android::base::ParseUint("1234", &s));
  ASSERT_EQ(1234u, s);

  ASSERT_TRUE(android::base::ParseUint("12", &i, 15u));
  ASSERT_EQ(12u, i);
  errno = 0;
  ASSERT_FALSE(android::base::ParseUint("-12", &i, 15u));
  ASSERT_EQ(EINVAL, errno);
  errno = 0;
  ASSERT_FALSE(android::base::ParseUint("16", &i, 15u));
  ASSERT_EQ(ERANGE, errno);

  errno = 0;
  ASSERT_FALSE(android::base::ParseUint<unsigned short>("x", nullptr));
  ASSERT_EQ(EINVAL, errno);
  errno = 0;
  ASSERT_FALSE(android::base::ParseUint<unsigned short>("123x", nullptr));
  ASSERT_EQ(EINVAL, errno);
  ASSERT_TRUE(android::base::ParseUint<unsigned short>("1234", nullptr));

  errno = 0;
  unsigned long long int lli;
  EXPECT_FALSE(android::base::ParseUint("-123", &lli));
  EXPECT_EQ(EINVAL, errno);
  errno = 0;
  EXPECT_FALSE(android::base::ParseUint("  -123", &lli));
  EXPECT_EQ(EINVAL, errno);
}

TEST(parseint, no_implicit_octal) {
  int i = 0;
  ASSERT_TRUE(android::base::ParseInt("0123", &i));
  ASSERT_EQ(123, i);

  unsigned int u = 0u;
  ASSERT_TRUE(android::base::ParseUint("0123", &u));
  ASSERT_EQ(123u, u);
}

TEST(parseint, explicit_hex) {
  int i = 0;
  ASSERT_TRUE(android::base::ParseInt("0x123", &i));
  ASSERT_EQ(0x123, i);
  i = 0;
  EXPECT_TRUE(android::base::ParseInt("  0x123", &i));
  EXPECT_EQ(0x123, i);

  unsigned int u = 0u;
  ASSERT_TRUE(android::base::ParseUint("0x123", &u));
  ASSERT_EQ(0x123u, u);
  u = 0u;
  EXPECT_TRUE(android::base::ParseUint("  0x123", &u));
  EXPECT_EQ(0x123u, u);
}

TEST(parseint, string) {
  int i = 0;
  ASSERT_TRUE(android::base::ParseInt(std::string("123"), &i));
  ASSERT_EQ(123, i);

  unsigned int u = 0u;
  ASSERT_TRUE(android::base::ParseUint(std::string("123"), &u));
  ASSERT_EQ(123u, u);
}

TEST(parseint, untouched_on_failure) {
  int i = 123;
  ASSERT_FALSE(android::base::ParseInt("456x", &i));
  ASSERT_EQ(123, i);

  unsigned int u = 123u;
  ASSERT_FALSE(android::base::ParseUint("456x", &u));
  ASSERT_EQ(123u, u);
}

TEST(parseint, ParseByteCount) {
  uint64_t i = 0;
  ASSERT_TRUE(android::base::ParseByteCount("123b", &i));
  ASSERT_EQ(123ULL, i);

  ASSERT_TRUE(android::base::ParseByteCount("8k", &i));
  ASSERT_EQ(8ULL * 1024, i);

  ASSERT_TRUE(android::base::ParseByteCount("8M", &i));
  ASSERT_EQ(8ULL * 1024 * 1024, i);

  ASSERT_TRUE(android::base::ParseByteCount("6g", &i));
  ASSERT_EQ(6ULL * 1024 * 1024 * 1024, i);

  ASSERT_TRUE(android::base::ParseByteCount("1T", &i));
  ASSERT_EQ(1ULL * 1024 * 1024 * 1024 * 1024, i);

  ASSERT_TRUE(android::base::ParseByteCount("2p", &i));
  ASSERT_EQ(2ULL * 1024 * 1024 * 1024 * 1024 * 1024, i);

  ASSERT_TRUE(android::base::ParseByteCount("4e", &i));
  ASSERT_EQ(4ULL * 1024 * 1024 * 1024 * 1024 * 1024 * 1024, i);
}

TEST(parseint, ParseByteCount_invalid_suffix) {
  unsigned u;
  ASSERT_FALSE(android::base::ParseByteCount("1x", &u));
}

TEST(parseint, ParseByteCount_overflow) {
  uint64_t u64;
  ASSERT_FALSE(android::base::ParseByteCount("4294967295E", &u64));

  uint16_t u16;
  ASSERT_TRUE(android::base::ParseByteCount("63k", &u16));
  ASSERT_EQ(63U * 1024, u16);
  ASSERT_TRUE(android::base::ParseByteCount("65535b", &u16));
  ASSERT_EQ(65535U, u16);
  ASSERT_FALSE(android::base::ParseByteCount("65k", &u16));
}
