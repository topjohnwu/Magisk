/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "android-base/unique_fd.h"

#include <utility>

#include <gtest/gtest.h>

extern void consume_unique_fd(android::base::unique_fd fd);

TEST(unique_fd, bugprone_use_after_move) {
  // Compile time test for clang-tidy's bugprone-use-after-move check.
  android::base::unique_fd ufd(open("/dev/null", O_RDONLY | O_CLOEXEC));
  consume_unique_fd(std::move(ufd));
  ufd.reset(open("/dev/null", O_RDONLY | O_CLOEXEC));
  ufd.get();
  consume_unique_fd(std::move(ufd));
}
