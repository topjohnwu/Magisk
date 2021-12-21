/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "android-base/function_ref.h"

#include <gtest/gtest.h>

#include <functional>
#include <string>

namespace android::base {

TEST(function_ref, Ctor) {
  // Making sure it can be constructed in all meaningful ways

  using EmptyFunc = function_ref<void()>;

  EmptyFunc f1([] {});

  struct Functor {
    void operator()() const {}
  };
  EmptyFunc f2(Functor{});
  Functor fctr;
  EmptyFunc f3(fctr);

  EmptyFunc f4(std::function<void()>([f1, f2, f3] {
    (void)f1;
    (void)f2;
    (void)f3;
  }));

  const std::function<void()> func = [] {};
  EmptyFunc f5(func);

  static_assert(sizeof(f1) <= 2 * sizeof(void*), "Too big function_ref");
  static_assert(sizeof(f2) <= 2 * sizeof(void*), "Too big function_ref");
  static_assert(sizeof(f3) <= 2 * sizeof(void*), "Too big function_ref");
  static_assert(sizeof(f4) <= 2 * sizeof(void*), "Too big function_ref");
  static_assert(sizeof(f5) <= 2 * sizeof(void*), "Too big function_ref");
}

TEST(function_ref, Call) {
  function_ref<int(int)> view = [](int i) { return i + 1; };
  EXPECT_EQ(1, view(0));
  EXPECT_EQ(-1, view(-2));

  function_ref<std::string(std::string)> fs = [](const std::string& s) { return s + "1"; };
  EXPECT_STREQ("s1", fs("s").c_str());
  EXPECT_STREQ("ssss1", fs("ssss").c_str());

  std::string base;
  auto lambda = [&base]() { return base + "1"; };
  function_ref<std::string()> fs2 = lambda;
  base = "one";
  EXPECT_STREQ("one1", fs2().c_str());
  base = "forty two";
  EXPECT_STREQ("forty two1", fs2().c_str());
}

TEST(function_ref, CopyAndAssign) {
  function_ref<int(int)> view = [](int i) { return i + 1; };
  EXPECT_EQ(1, view(0));
  view = [](int i) { return i - 1; };
  EXPECT_EQ(0, view(1));

  function_ref<int(int)> view2 = view;
  EXPECT_EQ(view(10), view2(10));

  view = view2;
  EXPECT_EQ(view(10), view2(10));
}

}  // namespace android::base
