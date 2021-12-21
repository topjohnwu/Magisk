/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "android-base/parsenetaddress.h"

#include <gtest/gtest.h>

using android::base::ParseNetAddress;

TEST(ParseNetAddressTest, TestUrl) {
  std::string canonical, host, error;
  int port = 123;

  EXPECT_TRUE(
      ParseNetAddress("www.google.com", &host, &port, &canonical, &error));
  EXPECT_EQ("www.google.com:123", canonical);
  EXPECT_EQ("www.google.com", host);
  EXPECT_EQ(123, port);

  EXPECT_TRUE(
      ParseNetAddress("www.google.com:666", &host, &port, &canonical, &error));
  EXPECT_EQ("www.google.com:666", canonical);
  EXPECT_EQ("www.google.com", host);
  EXPECT_EQ(666, port);
}

TEST(ParseNetAddressTest, TestIpv4) {
  std::string canonical, host, error;
  int port = 123;

  EXPECT_TRUE(ParseNetAddress("1.2.3.4", &host, &port, &canonical, &error));
  EXPECT_EQ("1.2.3.4:123", canonical);
  EXPECT_EQ("1.2.3.4", host);
  EXPECT_EQ(123, port);

  EXPECT_TRUE(ParseNetAddress("1.2.3.4:666", &host, &port, &canonical, &error));
  EXPECT_EQ("1.2.3.4:666", canonical);
  EXPECT_EQ("1.2.3.4", host);
  EXPECT_EQ(666, port);
}

TEST(ParseNetAddressTest, TestIpv6) {
  std::string canonical, host, error;
  int port = 123;

  EXPECT_TRUE(ParseNetAddress("::1", &host, &port, &canonical, &error));
  EXPECT_EQ("[::1]:123", canonical);
  EXPECT_EQ("::1", host);
  EXPECT_EQ(123, port);

  EXPECT_TRUE(ParseNetAddress("fe80::200:5aee:feaa:20a2", &host, &port,
                              &canonical, &error));
  EXPECT_EQ("[fe80::200:5aee:feaa:20a2]:123", canonical);
  EXPECT_EQ("fe80::200:5aee:feaa:20a2", host);
  EXPECT_EQ(123, port);

  EXPECT_TRUE(ParseNetAddress("[::1]:666", &host, &port, &canonical, &error));
  EXPECT_EQ("[::1]:666", canonical);
  EXPECT_EQ("::1", host);
  EXPECT_EQ(666, port);

  EXPECT_TRUE(ParseNetAddress("[fe80::200:5aee:feaa:20a2]:666", &host, &port,
                              &canonical, &error));
  EXPECT_EQ("[fe80::200:5aee:feaa:20a2]:666", canonical);
  EXPECT_EQ("fe80::200:5aee:feaa:20a2", host);
  EXPECT_EQ(666, port);
}

TEST(ParseNetAddressTest, TestInvalidAddress) {
  std::string canonical, host;
  int port;

  std::string failure_cases[] = {
      // Invalid IPv4.
      "1.2.3.4:",
      "1.2.3.4::",
      ":123",

      // Invalid IPv6.
      ":1",
      "::::::::1",
      "[::1",
      "[::1]",
      "[::1]:",
      "[::1]::",

      // Invalid port.
      "1.2.3.4:-1",
      "1.2.3.4:0",
      "1.2.3.4:65536"
      "1.2.3.4:hello",
      "[::1]:-1",
      "[::1]:0",
      "[::1]:65536",
      "[::1]:hello",
  };

  for (const auto& address : failure_cases) {
    // Failure should give some non-empty error string.
    std::string error;
    EXPECT_FALSE(ParseNetAddress(address, &host, &port, &canonical, &error));
    EXPECT_NE("", error);
  }
}

// Null canonical address argument.
TEST(ParseNetAddressTest, TestNullCanonicalAddress) {
  std::string host, error;
  int port = 42;

  EXPECT_TRUE(ParseNetAddress("www.google.com", &host, &port, nullptr, &error));
  EXPECT_TRUE(ParseNetAddress("1.2.3.4", &host, &port, nullptr, &error));
  EXPECT_TRUE(ParseNetAddress("::1", &host, &port, nullptr, &error));
}
