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

#include "android-base/scopeguard.h"

#include <utility>
#include <vector>

#include <gtest/gtest.h>

TEST(scopeguard, normal) {
  bool guarded_var = true;
  {
    auto scopeguard = android::base::make_scope_guard([&guarded_var] { guarded_var = false; });
  }
  ASSERT_FALSE(guarded_var);
}

TEST(scopeguard, disabled) {
  bool guarded_var = true;
  {
    auto scopeguard = android::base::make_scope_guard([&guarded_var] { guarded_var = false; });
    scopeguard.Disable();
  }
  ASSERT_TRUE(guarded_var);
}

TEST(scopeguard, moved) {
  int guarded_var = true;
  auto scopeguard = android::base::make_scope_guard([&guarded_var] { guarded_var = false; });
  { decltype(scopeguard) new_guard(std::move(scopeguard)); }
  EXPECT_FALSE(scopeguard.active());
  ASSERT_FALSE(guarded_var);
}

TEST(scopeguard, vector) {
  int guarded_var = 0;
  {
    std::vector<android::base::ScopeGuard<std::function<void()>>> scopeguards;
    scopeguards.emplace_back(android::base::make_scope_guard(
        std::bind([](int& guarded_var) { guarded_var++; }, std::ref(guarded_var))));
    scopeguards.emplace_back(android::base::make_scope_guard(
        std::bind([](int& guarded_var) { guarded_var++; }, std::ref(guarded_var))));
  }
  ASSERT_EQ(guarded_var, 2);
}
