/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <processgroup/sched_policy.h>

#define LOG_TAG "SchedPolicy"

#include <errno.h>
#include <unistd.h>

#include <android-base/logging.h>
#include <android-base/threads.h>
#include <cgroup_map.h>
#include <processgroup/processgroup.h>

using android::base::GetThreadId;

/* Re-map SP_DEFAULT to the system default policy, and leave other values unchanged.
 * Call this any place a SchedPolicy is used as an input parameter.
 * Returns the possibly re-mapped policy.
 */
static inline SchedPolicy _policy(SchedPolicy p) {
    return p == SP_DEFAULT ? SP_SYSTEM_DEFAULT : p;
}

#if defined(__ANDROID__)

int set_cpuset_policy(int tid, SchedPolicy policy) {
    if (tid == 0) {
        tid = GetThreadId();
    }
    policy = _policy(policy);

    switch (policy) {
        case SP_BACKGROUND:
            return SetTaskProfiles(tid, {"CPUSET_SP_BACKGROUND"}, true) ? 0 : -1;
        case SP_FOREGROUND:
        case SP_AUDIO_APP:
        case SP_AUDIO_SYS:
            return SetTaskProfiles(tid, {"CPUSET_SP_FOREGROUND"}, true) ? 0 : -1;
        case SP_TOP_APP:
            return SetTaskProfiles(tid, {"CPUSET_SP_TOP_APP"}, true) ? 0 : -1;
        case SP_SYSTEM:
            return SetTaskProfiles(tid, {"CPUSET_SP_SYSTEM"}, true) ? 0 : -1;
        case SP_RESTRICTED:
            return SetTaskProfiles(tid, {"CPUSET_SP_RESTRICTED"}, true) ? 0 : -1;
        default:
            break;
    }

    return 0;
}

int set_sched_policy(int tid, SchedPolicy policy) {
    if (tid == 0) {
        tid = GetThreadId();
    }
    policy = _policy(policy);

#if POLICY_DEBUG
    char statfile[64];
    char statline[1024];
    char thread_name[255];

    snprintf(statfile, sizeof(statfile), "/proc/%d/stat", tid);
    memset(thread_name, 0, sizeof(thread_name));

    unique_fd fd(TEMP_FAILURE_RETRY(open(statfile, O_RDONLY | O_CLOEXEC)));
    if (fd >= 0) {
        int rc = read(fd, statline, 1023);
        statline[rc] = 0;
        char* p = statline;
        char* q;

        for (p = statline; *p != '('; p++)
            ;
        p++;
        for (q = p; *q != ')'; q++)
            ;

        strncpy(thread_name, p, (q - p));
    }
    switch (policy) {
        case SP_BACKGROUND:
            SLOGD("vvv tid %d (%s)", tid, thread_name);
            break;
        case SP_FOREGROUND:
        case SP_AUDIO_APP:
        case SP_AUDIO_SYS:
        case SP_TOP_APP:
            SLOGD("^^^ tid %d (%s)", tid, thread_name);
            break;
        case SP_SYSTEM:
            SLOGD("/// tid %d (%s)", tid, thread_name);
            break;
        case SP_RT_APP:
            SLOGD("RT  tid %d (%s)", tid, thread_name);
            break;
        default:
            SLOGD("??? tid %d (%s)", tid, thread_name);
            break;
    }
#endif

    switch (policy) {
        case SP_BACKGROUND:
            return SetTaskProfiles(tid, {"SCHED_SP_BACKGROUND"}, true) ? 0 : -1;
        case SP_FOREGROUND:
        case SP_AUDIO_APP:
        case SP_AUDIO_SYS:
            return SetTaskProfiles(tid, {"SCHED_SP_FOREGROUND"}, true) ? 0 : -1;
        case SP_TOP_APP:
            return SetTaskProfiles(tid, {"SCHED_SP_TOP_APP"}, true) ? 0 : -1;
        case SP_SYSTEM:
            return SetTaskProfiles(tid, {"SCHED_SP_SYSTEM"}, true) ? 0 : -1;
        case SP_RT_APP:
            return SetTaskProfiles(tid, {"SCHED_SP_RT_APP"}, true) ? 0 : -1;
        default:
            return SetTaskProfiles(tid, {"SCHED_SP_DEFAULT"}, true) ? 0 : -1;
    }

    return 0;
}

bool cpusets_enabled() {
    static bool enabled = (CgroupMap::GetInstance().FindController("cpuset").IsUsable());
    return enabled;
}

static bool schedtune_enabled() {
    return (CgroupMap::GetInstance().FindController("schedtune").IsUsable());
}

static bool cpuctl_enabled() {
    return (CgroupMap::GetInstance().FindController("cpu").IsUsable());
}

bool schedboost_enabled() {
    static bool enabled = schedtune_enabled() || cpuctl_enabled();

    return enabled;
}

static int getCGroupSubsys(int tid, const char* subsys, std::string& subgroup) {
    auto controller = CgroupMap::GetInstance().FindController(subsys);

    if (!controller.IsUsable()) return -1;

    if (!controller.GetTaskGroup(tid, &subgroup))
        return -1;

    return 0;
}

int get_sched_policy(int tid, SchedPolicy* policy) {
    if (tid == 0) {
        tid = GetThreadId();
    }

    std::string group;
    if (schedboost_enabled()) {
        if ((getCGroupSubsys(tid, "schedtune", group) < 0) &&
            (getCGroupSubsys(tid, "cpu", group) < 0)) {
                LOG(ERROR) << "Failed to find cpu cgroup for tid " << tid;
                return -1;
        }
    }
    if (group.empty() && cpusets_enabled()) {
        if (getCGroupSubsys(tid, "cpuset", group) < 0) {
            LOG(ERROR) << "Failed to find cpuset cgroup for tid " << tid;
            return -1;
        }
    }

    // TODO: replace hardcoded directories
    if (group.empty()) {
        *policy = SP_FOREGROUND;
    } else if (group == "foreground") {
        *policy = SP_FOREGROUND;
    } else if (group == "system-background") {
        *policy = SP_SYSTEM;
    } else if (group == "background") {
        *policy = SP_BACKGROUND;
    } else if (group == "top-app") {
        *policy = SP_TOP_APP;
    } else if (group == "restricted") {
        *policy = SP_RESTRICTED;
    } else {
        errno = ERANGE;
        return -1;
    }
    return 0;
}

#else

/* Stubs for non-Android targets. */

int set_sched_policy(int, SchedPolicy) {
    return 0;
}

int get_sched_policy(int, SchedPolicy* policy) {
    *policy = SP_SYSTEM_DEFAULT;
    return 0;
}

#endif

const char* get_sched_policy_name(SchedPolicy policy) {
    policy = _policy(policy);
    static const char* const kSchedPolicyNames[] = {
            [SP_BACKGROUND] = "bg", [SP_FOREGROUND] = "fg", [SP_SYSTEM] = "  ",
            [SP_AUDIO_APP] = "aa",  [SP_AUDIO_SYS] = "as",  [SP_TOP_APP] = "ta",
            [SP_RT_APP] = "rt",     [SP_RESTRICTED] = "rs",
    };
    static_assert(arraysize(kSchedPolicyNames) == SP_CNT, "missing name");
    if (policy < SP_BACKGROUND || policy >= SP_CNT) {
        return nullptr;
    }
    return kSchedPolicyNames[policy];
}

const char* get_cpuset_policy_profile_name(SchedPolicy policy) {
    /*
     *  cpuset profile array for:
     *  SP_DEFAULT(-1), SP_BACKGROUND(0), SP_FOREGROUND(1),
     *  SP_SYSTEM(2), SP_AUDIO_APP(3), SP_AUDIO_SYS(4),
     *  SP_TOP_APP(5), SP_RT_APP(6), SP_RESTRICTED(7)
     *  index is policy + 1
     *  this need keep in sync with SchedPolicy enum
     */
    static constexpr const char* kCpusetProfiles[SP_CNT + 1] = {
            "CPUSET_SP_DEFAULT", "CPUSET_SP_BACKGROUND", "CPUSET_SP_FOREGROUND",
            "CPUSET_SP_SYSTEM",  "CPUSET_SP_FOREGROUND", "CPUSET_SP_FOREGROUND",
            "CPUSET_SP_TOP_APP", "CPUSET_SP_DEFAULT",    "CPUSET_SP_RESTRICTED"};
    if (policy < SP_DEFAULT || policy >= SP_CNT) {
        return nullptr;
    }
    return kCpusetProfiles[policy + 1];
}

const char* get_sched_policy_profile_name(SchedPolicy policy) {
    /*
     *  sched profile array for:
     *  SP_DEFAULT(-1), SP_BACKGROUND(0), SP_FOREGROUND(1),
     *  SP_SYSTEM(2), SP_AUDIO_APP(3), SP_AUDIO_SYS(4),
     *  SP_TOP_APP(5), SP_RT_APP(6), SP_RESTRICTED(7)
     *  index is policy + 1
     *  this need keep in sync with SchedPolicy enum
     */
    static constexpr const char* kSchedProfiles[SP_CNT + 1] = {
            "SCHED_SP_DEFAULT", "SCHED_SP_BACKGROUND", "SCHED_SP_FOREGROUND",
            "SCHED_SP_SYSTEM",  "SCHED_SP_FOREGROUND", "SCHED_SP_FOREGROUND",
            "SCHED_SP_TOP_APP", "SCHED_SP_RT_APP",     "SCHED_SP_DEFAULT"};
    if (policy < SP_DEFAULT || policy >= SP_CNT) {
        return nullptr;
    }
    return kSchedProfiles[policy + 1];
}
