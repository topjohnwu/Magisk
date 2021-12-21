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

namespace android {
namespace base {

boot_clock::time_point boot_clock::now() {
#ifdef __linux__
  timespec ts;
  clock_gettime(CLOCK_BOOTTIME, &ts);
  return boot_clock::time_point(std::chrono::seconds(ts.tv_sec) +
                                std::chrono::nanoseconds(ts.tv_nsec));
#else
  // Darwin and Windows do not support clock_gettime.
  return boot_clock::time_point();
#endif  // __linux__
}

std::ostream& operator<<(std::ostream& os, const Timer& t) {
  os << t.duration().count() << "ms";
  return os;
}

}  // namespace base
}  // namespace android
