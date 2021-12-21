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

#include "android-base/chrono_utils.h"

#include <time.h>

#include <chrono>
#include <sstream>
#include <string>
#include <thread>

#include <gtest/gtest.h>

namespace android {
namespace base {

std::chrono::seconds GetBootTimeSeconds() {
  struct timespec now;
  clock_gettime(CLOCK_BOOTTIME, &now);

  auto now_tp = boot_clock::time_point(std::chrono::seconds(now.tv_sec) +
                                       std::chrono::nanoseconds(now.tv_nsec));
  return std::chrono::duration_cast<std::chrono::seconds>(now_tp.time_since_epoch());
}

// Tests (at least) the seconds accuracy of the boot_clock::now() method.
TEST(ChronoUtilsTest, BootClockNowSeconds) {
  auto now = GetBootTimeSeconds();
  auto boot_seconds =
      std::chrono::duration_cast<std::chrono::seconds>(boot_clock::now().time_since_epoch());
  EXPECT_EQ(now, boot_seconds);
}

template <typename T>
void ExpectAboutEqual(T expected, T actual) {
  auto expected_upper_bound = expected * 1.05f;
  auto expected_lower_bound = expected * .95;
  EXPECT_GT(expected_upper_bound, actual);
  EXPECT_LT(expected_lower_bound, actual);
}

TEST(ChronoUtilsTest, TimerDurationIsSane) {
  auto start = boot_clock::now();
  Timer t;
  std::this_thread::sleep_for(50ms);
  auto stop = boot_clock::now();
  auto stop_timer = t.duration();

  auto expected = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  ExpectAboutEqual(expected, stop_timer);
}

TEST(ChronoUtilsTest, TimerOstream) {
  Timer t;
  std::this_thread::sleep_for(50ms);
  auto stop_timer = t.duration().count();
  std::stringstream os;
  os << t;
  decltype(stop_timer) stop_timer_from_stream;
  os >> stop_timer_from_stream;
  EXPECT_NE(0, stop_timer);
  ExpectAboutEqual(stop_timer, stop_timer_from_stream);
}

}  // namespace base
}  // namespace android
