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

#include "android-base/endian.h"

#include <gtest/gtest.h>

TEST(endian, constants) {
  ASSERT_TRUE(__LITTLE_ENDIAN == LITTLE_ENDIAN);
  ASSERT_TRUE(__BIG_ENDIAN == BIG_ENDIAN);
  ASSERT_TRUE(__BYTE_ORDER == BYTE_ORDER);

  ASSERT_EQ(__LITTLE_ENDIAN, __BYTE_ORDER);
}

TEST(endian, smoke) {
  static constexpr uint16_t le16 = 0x1234;
  static constexpr uint32_t le32 = 0x12345678;
  static constexpr uint64_t le64 = 0x123456789abcdef0;

  static constexpr uint16_t be16 = 0x3412;
  static constexpr uint32_t be32 = 0x78563412;
  static constexpr uint64_t be64 = 0xf0debc9a78563412;

  ASSERT_EQ(be16, htons(le16));
  ASSERT_EQ(be32, htonl(le32));
  ASSERT_EQ(be64, htonq(le64));

  ASSERT_EQ(le16, ntohs(be16));
  ASSERT_EQ(le32, ntohl(be32));
  ASSERT_EQ(le64, ntohq(be64));

  ASSERT_EQ(be16, htobe16(le16));
  ASSERT_EQ(be32, htobe32(le32));
  ASSERT_EQ(be64, htobe64(le64));

  ASSERT_EQ(le16, betoh16(be16));
  ASSERT_EQ(le32, betoh32(be32));
  ASSERT_EQ(le64, betoh64(be64));

  ASSERT_EQ(le16, htole16(le16));
  ASSERT_EQ(le32, htole32(le32));
  ASSERT_EQ(le64, htole64(le64));

  ASSERT_EQ(le16, letoh16(le16));
  ASSERT_EQ(le32, letoh32(le32));
  ASSERT_EQ(le64, letoh64(le64));

  ASSERT_EQ(le16, be16toh(be16));
  ASSERT_EQ(le32, be32toh(be32));
  ASSERT_EQ(le64, be64toh(be64));

  ASSERT_EQ(le16, le16toh(le16));
  ASSERT_EQ(le32, le32toh(le32));
  ASSERT_EQ(le64, le64toh(le64));
}
