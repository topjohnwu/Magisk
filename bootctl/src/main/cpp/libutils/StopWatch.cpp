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

#define LOG_TAG "StopWatch"

#include <utils/StopWatch.h>

/* for PRId64 */
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
#include <inttypes.h>

#include <utils/Log.h>

namespace android {

StopWatch::StopWatch(const char* name, int clock) : mName(name), mClock(clock) {
    reset();
}

StopWatch::~StopWatch() {
    ALOGD("StopWatch %s (us): %" PRId64 " ", name(), ns2us(elapsedTime()));
}

const char* StopWatch::name() const {
    return mName;
}

nsecs_t StopWatch::elapsedTime() const {
    return systemTime(mClock) - mStartTime;
}

void StopWatch::reset() {
    mStartTime = systemTime(mClock);
}

}  // namespace android
