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

#include <utils/Timers.h>

#include <gtest/gtest.h>

TEST(Timers, systemTime_invalid) {
    EXPECT_EXIT(systemTime(-1), testing::KilledBySignal(SIGABRT), "");
    systemTime(SYSTEM_TIME_REALTIME);
    systemTime(SYSTEM_TIME_MONOTONIC);
    systemTime(SYSTEM_TIME_PROCESS);
    systemTime(SYSTEM_TIME_THREAD);
    systemTime(SYSTEM_TIME_BOOTTIME);
    EXPECT_EXIT(systemTime(SYSTEM_TIME_BOOTTIME + 1), testing::KilledBySignal(SIGABRT), "");
}
