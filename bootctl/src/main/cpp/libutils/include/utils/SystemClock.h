/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_UTILS_SYSTEMCLOCK_H
#define ANDROID_UTILS_SYSTEMCLOCK_H

#include <stdint.h>
#include <sys/types.h>

// See https://developer.android.com/reference/android/os/SystemClock
// to learn more about Android's timekeeping facilities.

namespace android {

// Returns milliseconds since boot, not counting time spent in deep sleep.
int64_t uptimeMillis();
// Returns nanoseconds since boot, not counting time spent in deep sleep.
int64_t uptimeNanos();
// Returns milliseconds since boot, including time spent in sleep.
int64_t elapsedRealtime();
// Returns nanoseconds since boot, including time spent in sleep.
int64_t elapsedRealtimeNano();

}  // namespace android

#endif // ANDROID_UTILS_SYSTEMCLOCK_H

