/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "android-base/macros.h"

#include <stdint.h>

#include <gtest/gtest.h>

TEST(macros, SIZEOF_MEMBER_macro) {
  struct S {
    int32_t i32;
    double d;
  };
  ASSERT_EQ(4U, SIZEOF_MEMBER(S, i32));
  ASSERT_EQ(8U, SIZEOF_MEMBER(S, d));
}
