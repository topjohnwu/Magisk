/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <unistd.h>
#include <utils/SystemClock.h>

#include <gtest/gtest.h>

static const auto MS_IN_NS = 1000000;

static const int64_t SLEEP_MS = 500;
static const int64_t SLEEP_NS = SLEEP_MS * MS_IN_NS;
// Conservatively assume that we might be descheduled for up to 50 ms
static const int64_t SLACK_MS = 50;
static const int64_t SLACK_NS = SLACK_MS * MS_IN_NS;

TEST(SystemClock, SystemClock) {
    auto startUptimeMs = android::uptimeMillis();
    auto startUptimeNs = android::uptimeNanos();
    auto startRealtimeMs = android::elapsedRealtime();
    auto startRealtimeNs = android::elapsedRealtimeNano();

    ASSERT_GT(startUptimeMs, 0)
            << "uptimeMillis() reported an impossible uptime";
    ASSERT_GT(startUptimeNs, 0)
            << "uptimeNanos() reported an impossible uptime";
    ASSERT_GE(startRealtimeMs, startUptimeMs)
            << "elapsedRealtime() thinks we've suspended for negative time";
    ASSERT_GE(startRealtimeNs, startUptimeNs)
            << "elapsedRealtimeNano() thinks we've suspended for negative time";

    ASSERT_GE(startUptimeNs, startUptimeMs * MS_IN_NS)
            << "uptimeMillis() and uptimeNanos() are inconsistent";
    ASSERT_LT(startUptimeNs, (startUptimeMs + SLACK_MS) * MS_IN_NS)
            << "uptimeMillis() and uptimeNanos() are inconsistent";

    ASSERT_GE(startRealtimeNs, startRealtimeMs * MS_IN_NS)
            << "elapsedRealtime() and elapsedRealtimeNano() are inconsistent";
    ASSERT_LT(startRealtimeNs, (startRealtimeMs + SLACK_MS) * MS_IN_NS)
            << "elapsedRealtime() and elapsedRealtimeNano() are inconsistent";

    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = SLEEP_MS * MS_IN_NS;
    auto nanosleepErr = TEMP_FAILURE_RETRY(nanosleep(&ts, nullptr));
    ASSERT_EQ(nanosleepErr, 0) << "nanosleep() failed: " << strerror(errno);

    auto endUptimeMs = android::uptimeMillis();
    auto endUptimeNs = android::uptimeNanos();
    auto endRealtimeMs = android::elapsedRealtime();
    auto endRealtimeNs = android::elapsedRealtimeNano();

    EXPECT_GE(endUptimeMs - startUptimeMs, SLEEP_MS)
            << "uptimeMillis() advanced too little after nanosleep()";
    EXPECT_LT(endUptimeMs - startUptimeMs, SLEEP_MS + SLACK_MS)
            << "uptimeMillis() advanced too much after nanosleep()";
    EXPECT_GE(endUptimeNs - startUptimeNs, SLEEP_NS)
            << "uptimeNanos() advanced too little after nanosleep()";
    EXPECT_LT(endUptimeNs - startUptimeNs, SLEEP_NS + SLACK_NS)
            << "uptimeNanos() advanced too much after nanosleep()";
    EXPECT_GE(endRealtimeMs - startRealtimeMs, SLEEP_MS)
            << "elapsedRealtime() advanced too little after nanosleep()";
    EXPECT_LT(endRealtimeMs - startRealtimeMs, SLEEP_MS + SLACK_MS)
            << "elapsedRealtime() advanced too much after nanosleep()";
    EXPECT_GE(endRealtimeNs - startRealtimeNs, SLEEP_NS)
            << "elapsedRealtimeNano() advanced too little after nanosleep()";
    EXPECT_LT(endRealtimeNs - startRealtimeNs, SLEEP_NS + SLACK_NS)
            << "elapsedRealtimeNano() advanced too much after nanosleep()";

    EXPECT_GE(endUptimeNs, endUptimeMs * MS_IN_NS)
            << "uptimeMillis() and uptimeNanos() are inconsistent after nanosleep()";
    EXPECT_LT(endUptimeNs, (endUptimeMs + SLACK_MS) * MS_IN_NS)
            << "uptimeMillis() and uptimeNanos() are inconsistent after nanosleep()";

    EXPECT_GE(endRealtimeNs, endRealtimeMs * MS_IN_NS)
            << "elapsedRealtime() and elapsedRealtimeNano() are inconsistent after nanosleep()";
    EXPECT_LT(endRealtimeNs, (endRealtimeMs + SLACK_MS) * MS_IN_NS)
            << "elapsedRealtime() and elapsedRealtimeNano() are inconsistent after nanosleep()";
}
