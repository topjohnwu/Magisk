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

#include "android-base/process.h"

#include <unistd.h>

#include <gtest/gtest.h>

TEST(process, find_ourselves) {
#if defined(__linux__)
  bool found_our_pid = false;
  for (const auto& pid : android::base::AllPids{}) {
    if (pid == getpid()) {
      found_our_pid = true;
    }
  }

  EXPECT_TRUE(found_our_pid);

#endif
}
