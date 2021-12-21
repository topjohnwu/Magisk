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

#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

#include <sys/capability.h>

#include <cutils/sched_policy.h>

#include <gtest/gtest.h>

bool hasCapSysNice() {
    __user_cap_header_struct header;
    memset(&header, 0, sizeof(header));
    header.version = _LINUX_CAPABILITY_VERSION_3;

    __user_cap_data_struct caps[_LINUX_CAPABILITY_U32S_3];
    if (capget(&header, &caps[0])) {
        GTEST_LOG_(WARNING) << "failed to get process capabilities";
        return false;
    }

    auto nice_idx = CAP_TO_INDEX(CAP_SYS_NICE);
    auto nice_mask = CAP_TO_MASK(CAP_SYS_NICE);
    return caps[nice_idx].effective & nice_mask;
}

long long medianSleepTime() {
    std::vector<long long> sleepTimes;
    constexpr size_t numSamples = 100;

    for (size_t i = 0; i < numSamples; i++) {
        auto start = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        auto end = std::chrono::steady_clock::now();

        auto diff = end - start;
        sleepTimes.push_back(diff.count());
    }

    constexpr auto median = numSamples / 2;
    std::nth_element(sleepTimes.begin(), sleepTimes.begin() + median,
            sleepTimes.end());
    return sleepTimes[median];
}

static void AssertPolicy(SchedPolicy expected_policy) {
    SchedPolicy current_policy;
    ASSERT_EQ(0, get_sched_policy(0, &current_policy));
    EXPECT_EQ(expected_policy, current_policy);
}

TEST(SchedPolicy, set_sched_policy) {
    if (!schedboost_enabled()) {
        // schedboost_enabled() (i.e. CONFIG_CGROUP_SCHEDTUNE) is optional;
        // it's only needed on devices using energy-aware scheduler.
        GTEST_LOG_(INFO) << "skipping test that requires CONFIG_CGROUP_SCHEDTUNE";
        return;
    }

    ASSERT_EQ(0, set_sched_policy(0, SP_BACKGROUND));
    AssertPolicy(SP_BACKGROUND);

    ASSERT_EQ(0, set_sched_policy(0, SP_FOREGROUND));
    AssertPolicy(SP_FOREGROUND);
}

TEST(SchedPolicy, set_sched_policy_timerslack) {
    if (!hasCapSysNice()) {
        GTEST_LOG_(INFO) << "skipping test that requires CAP_SYS_NICE";
        return;
    }

    // A measureable effect of scheduling policy is that the kernel has 800x
    // greater slack time in waking up a sleeping background thread.
    //
    // Look for 10x difference in how long FB and BG threads actually sleep
    // when trying to sleep for 1 ns.  This difference is large enough not
    // to happen by chance, but small enough (compared to 800x) to keep inherent
    // fuzziness in scheduler behavior from causing false negatives.
    const unsigned int BG_FG_SLACK_FACTOR = 10;

    ASSERT_EQ(0, set_sched_policy(0, SP_BACKGROUND));
    auto bgSleepTime = medianSleepTime();

    ASSERT_EQ(0, set_sched_policy(0, SP_FOREGROUND));
    auto fgSleepTime = medianSleepTime();

    ASSERT_GT(bgSleepTime, fgSleepTime * BG_FG_SLACK_FACTOR);
}

TEST(SchedPolicy, get_sched_policy_name) {
    EXPECT_STREQ("bg", get_sched_policy_name(SP_BACKGROUND));
    EXPECT_EQ(nullptr, get_sched_policy_name(SchedPolicy(-2)));
    EXPECT_EQ(nullptr, get_sched_policy_name(SP_CNT));
}

TEST(SchedPolicy, get_cpuset_policy_profile_name) {
    EXPECT_STREQ("CPUSET_SP_BACKGROUND", get_cpuset_policy_profile_name(SP_BACKGROUND));
    EXPECT_EQ(nullptr, get_cpuset_policy_profile_name(SchedPolicy(-2)));
    EXPECT_EQ(nullptr, get_cpuset_policy_profile_name(SP_CNT));
}

TEST(SchedPolicy, get_sched_policy_profile_name) {
    EXPECT_STREQ("SCHED_SP_BACKGROUND", get_sched_policy_profile_name(SP_BACKGROUND));
    EXPECT_EQ(nullptr, get_sched_policy_profile_name(SchedPolicy(-2)));
    EXPECT_EQ(nullptr, get_sched_policy_profile_name(SP_CNT));
}
