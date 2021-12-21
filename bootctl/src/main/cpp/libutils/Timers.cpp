/*
 * Copyright (C) 2005 The Android Open Source Project
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

//
// Timer functions.
//
#include <utils/Timers.h>

#include <limits.h>
#include <stdlib.h>
#include <time.h>

#include <android-base/macros.h>

static constexpr size_t clock_id_max = 5;

static void checkClockId(int clock) {
    if (clock < 0 || clock >= clock_id_max) abort();
}

#if defined(__linux__)
nsecs_t systemTime(int clock) {
    checkClockId(clock);
    static constexpr clockid_t clocks[] = {CLOCK_REALTIME, CLOCK_MONOTONIC,
                                           CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID,
                                           CLOCK_BOOTTIME};
    static_assert(clock_id_max == arraysize(clocks));
    timespec t = {};
    clock_gettime(clocks[clock], &t);
    return nsecs_t(t.tv_sec)*1000000000LL + t.tv_nsec;
}
#else
nsecs_t systemTime(int clock) {
    // TODO: is this ever called with anything but REALTIME on mac/windows?
    checkClockId(clock);

    // Clock support varies widely across hosts. Mac OS doesn't support
    // CLOCK_BOOTTIME (and doesn't even have clock_gettime until 10.12).
    // Windows is windows.
    timeval t = {};
    gettimeofday(&t, nullptr);
    return nsecs_t(t.tv_sec)*1000000000LL + nsecs_t(t.tv_usec)*1000LL;
}
#endif

int toMillisecondTimeoutDelay(nsecs_t referenceTime, nsecs_t timeoutTime)
{
    nsecs_t timeoutDelayMillis;
    if (timeoutTime > referenceTime) {
        uint64_t timeoutDelay = uint64_t(timeoutTime - referenceTime);
        if (timeoutDelay > uint64_t((INT_MAX - 1) * 1000000LL)) {
            timeoutDelayMillis = -1;
        } else {
            timeoutDelayMillis = (timeoutDelay + 999999LL) / 1000000LL;
        }
    } else {
        timeoutDelayMillis = 0;
    }
    return (int)timeoutDelayMillis;
}
