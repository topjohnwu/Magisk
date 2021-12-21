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


/*
 * System clock functions.
 */

#define LOG_TAG "SystemClock"

#include <utils/SystemClock.h>

#include <string.h>
#include <errno.h>
#include <time.h>

#include <cutils/compiler.h>

#include <utils/Timers.h>
#include <utils/Log.h>

namespace android {

/*
 * native public static long uptimeMillis();
 */
int64_t uptimeMillis()
{
    return nanoseconds_to_milliseconds(uptimeNanos());
}

/*
 * public static native long uptimeNanos();
 */
int64_t uptimeNanos()
{
    return systemTime(SYSTEM_TIME_MONOTONIC);
}

/*
 * native public static long elapsedRealtime();
 */
int64_t elapsedRealtime()
{
	return nanoseconds_to_milliseconds(elapsedRealtimeNano());
}

/*
 * native public static long elapsedRealtimeNano();
 */
int64_t elapsedRealtimeNano()
{
#if defined(__linux__)
    struct timespec ts;
    int err = clock_gettime(CLOCK_BOOTTIME, &ts);
    if (CC_UNLIKELY(err)) {
        // This should never happen, but just in case ...
        ALOGE("clock_gettime(CLOCK_BOOTTIME) failed: %s", strerror(errno));
        return 0;
    }

    return seconds_to_nanoseconds(ts.tv_sec) + ts.tv_nsec;
#else
    return systemTime(SYSTEM_TIME_MONOTONIC);
#endif
}

}; // namespace android
