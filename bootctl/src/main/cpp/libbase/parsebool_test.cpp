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

#include "android-base/parsebool.h"

#include <errno.h>

#include <gtest/gtest.h>
#include <string_view>

using android::base::ParseBool;
using android::base::ParseBoolResult;

TEST(parsebool, true_) {
  static const char* yes[] = {
      "1", "on", "true", "y", "yes",
  };
  for (const char* s : yes) {
    ASSERT_EQ(ParseBoolResult::kTrue, ParseBool(s));
  }
}

TEST(parsebool, false_) {
  static const char* no[] = {
      "0", "false", "n", "no", "off",
  };
  for (const char* s : no) {
    ASSERT_EQ(ParseBoolResult::kFalse, ParseBool(s));
  }
}

TEST(parsebool, invalid) {
  ASSERT_EQ(ParseBoolResult::kError, ParseBool("blarg"));
  ASSERT_EQ(ParseBoolResult::kError, ParseBool(""));
}
