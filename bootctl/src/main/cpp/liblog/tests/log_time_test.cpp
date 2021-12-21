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

#include <time.h>

#include <gtest/gtest.h>
// Test the APIs in this standalone include file
#include <log/log_time.h>

TEST(liblog, log_time) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  log_time tl(ts);

  EXPECT_EQ(tl, ts);
  EXPECT_GE(tl, ts);
  EXPECT_LE(tl, ts);
}
