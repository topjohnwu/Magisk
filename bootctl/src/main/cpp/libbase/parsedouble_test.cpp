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

#include "android-base/parsedouble.h"

#include <gtest/gtest.h>

TEST(parsedouble, double_smoke) {
  double d;
  ASSERT_FALSE(android::base::ParseDouble("", &d));
  ASSERT_FALSE(android::base::ParseDouble("x", &d));
  ASSERT_FALSE(android::base::ParseDouble("123.4x", &d));

  ASSERT_TRUE(android::base::ParseDouble("123.4", &d));
  ASSERT_DOUBLE_EQ(123.4, d);
  ASSERT_TRUE(android::base::ParseDouble("-123.4", &d));
  ASSERT_DOUBLE_EQ(-123.4, d);

  ASSERT_TRUE(android::base::ParseDouble("0", &d, 0.0));
  ASSERT_DOUBLE_EQ(0.0, d);
  ASSERT_FALSE(android::base::ParseDouble("0", &d, 1e-9));
  ASSERT_FALSE(android::base::ParseDouble("3.0", &d, -1.0, 2.0));
  ASSERT_TRUE(android::base::ParseDouble("1.0", &d, 0.0, 2.0));
  ASSERT_DOUBLE_EQ(1.0, d);

  ASSERT_FALSE(android::base::ParseDouble("123.4x", nullptr));
  ASSERT_TRUE(android::base::ParseDouble("-123.4", nullptr));
  ASSERT_FALSE(android::base::ParseDouble("3.0", nullptr, -1.0, 2.0));
  ASSERT_TRUE(android::base::ParseDouble("1.0", nullptr, 0.0, 2.0));
}

TEST(parsedouble, float_smoke) {
  float f;
  ASSERT_FALSE(android::base::ParseFloat("", &f));
  ASSERT_FALSE(android::base::ParseFloat("x", &f));
  ASSERT_FALSE(android::base::ParseFloat("123.4x", &f));

  ASSERT_TRUE(android::base::ParseFloat("123.4", &f));
  ASSERT_FLOAT_EQ(123.4, f);
  ASSERT_TRUE(android::base::ParseFloat("-123.4", &f));
  ASSERT_FLOAT_EQ(-123.4, f);

  ASSERT_TRUE(android::base::ParseFloat("0", &f, 0.0));
  ASSERT_FLOAT_EQ(0.0, f);
  ASSERT_FALSE(android::base::ParseFloat("0", &f, 1e-9));
  ASSERT_FALSE(android::base::ParseFloat("3.0", &f, -1.0, 2.0));
  ASSERT_TRUE(android::base::ParseFloat("1.0", &f, 0.0, 2.0));
  ASSERT_FLOAT_EQ(1.0, f);

  ASSERT_FALSE(android::base::ParseFloat("123.4x", nullptr));
  ASSERT_TRUE(android::base::ParseFloat("-123.4", nullptr));
  ASSERT_FALSE(android::base::ParseFloat("3.0", nullptr, -1.0, 2.0));
  ASSERT_TRUE(android::base::ParseFloat("1.0", nullptr, 0.0, 2.0));
}
