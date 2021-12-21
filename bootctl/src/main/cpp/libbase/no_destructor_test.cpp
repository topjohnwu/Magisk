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

#include "android-base/no_destructor.h"

#include <gtest/gtest.h>

struct __attribute__((packed)) Bomb {
  Bomb() : magic_(123) {}

  ~Bomb() { exit(42); }

  int get() const { return magic_; }

 private:
  [[maybe_unused]] char padding_;
  int magic_;
};

TEST(no_destructor, bomb) {
  ASSERT_EXIT(({
                {
                  Bomb b;
                  if (b.get() != 123) exit(1);
                }

                exit(0);
              }),
              ::testing::ExitedWithCode(42), "");
}

TEST(no_destructor, defused) {
  ASSERT_EXIT(({
                {
                  android::base::NoDestructor<Bomb> b;
                  if (b->get() != 123) exit(1);
                }

                exit(0);
              }),
              ::testing::ExitedWithCode(0), "");
}

TEST(no_destructor, operators) {
  android::base::NoDestructor<Bomb> b;
  const android::base::NoDestructor<Bomb>& c = b;
  ASSERT_EQ(123, b.get()->get());
  ASSERT_EQ(123, b->get());
  ASSERT_EQ(123, (*b).get());
  ASSERT_EQ(123, c.get()->get());
  ASSERT_EQ(123, c->get());
  ASSERT_EQ(123, (*c).get());
}
