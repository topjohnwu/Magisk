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
#include <hidl/HidlTransportSupport.h>

#include <hidl/HidlBinderSupport.h>
#include "InternalStatic.h"

#include <android-base/logging.h>
#include <android/hidl/manager/1.0/IServiceManager.h>

#include <linux/sched.h>

namespace android {
namespace hardware {

using ::android::hidl::base::V1_0::IBase;

void configureRpcThreadpool(size_t maxThreads, bool callerWillJoin) {
    // TODO(b/32756130) this should be transport-dependent
    configureBinderRpcThreadpool(maxThreads, callerWillJoin);
}
void joinRpcThreadpool() {
    // TODO(b/32756130) this should be transport-dependent
    joinBinderRpcThreadpool();
}

int setupTransportPolling() {
    return setupBinderPolling();
}

status_t handleTransportPoll(int /*fd*/) {
    return handleBinderPoll();
}

// TODO(b/122472540): only store one data item per object
template <typename V>
static void pruneMapLocked(ConcurrentMap<wp<IBase>, V>& map) {
    std::vector<wp<IBase>> toDelete;
    for (const auto& kv : map) {
        if (kv.first.promote() == nullptr) {
            toDelete.push_back(kv.first);
        }
    }
    for (const auto& k : toDelete) {
        map.eraseLocked(k);
    }
}

bool setMinSchedulerPolicy(const sp<IBase>& service, int policy, int priority) {
    if (service->isRemote()) {
        LOG(ERROR) << "Can't set scheduler policy on remote service.";
        return false;
    }

    switch (policy) {
        case SCHED_NORMAL: {
            if (priority < -20 || priority > 19) {
                LOG(ERROR) << "Invalid priority for SCHED_NORMAL: " << priority;
                return false;
            }
        } break;
        case SCHED_RR:
        case SCHED_FIFO: {
            if (priority < 1 || priority > 99) {
                LOG(ERROR) << "Invalid priority for " << policy << " policy: " << priority;
                return false;
            }
        } break;
        default: {
            LOG(ERROR) << "Invalid scheduler policy " << policy;
            return false;
        }
    }

    // Due to ABI considerations, IBase cannot have a destructor to clean this up.
    // So, because this API is so infrequently used, (expected to be usually only
    // one time for a process, but it can be more), we are cleaning it up here.
    std::unique_lock<std::mutex> lock = details::gServicePrioMap->lock();
    pruneMapLocked(details::gServicePrioMap.get());
    details::gServicePrioMap->setLocked(service, {policy, priority});

    return true;
}

SchedPrio getMinSchedulerPolicy(const sp<IBase>& service) {
    return details::gServicePrioMap->get(service, {SCHED_NORMAL, 0});
}

bool setRequestingSid(const sp<IBase>& service, bool requesting) {
    if (service->isRemote()) {
        LOG(ERROR) << "Can't set requesting sid on remote service.";
        return false;
    }

    // Due to ABI considerations, IBase cannot have a destructor to clean this up.
    // So, because this API is so infrequently used, (expected to be usually only
    // one time for a process, but it can be more), we are cleaning it up here.
    std::unique_lock<std::mutex> lock = details::gServiceSidMap->lock();
    pruneMapLocked(details::gServiceSidMap.get());
    details::gServiceSidMap->setLocked(service, requesting);

    return true;
}

bool getRequestingSid(const sp<IBase>& service) {
    return details::gServiceSidMap->get(service.get(), false);
}

bool interfacesEqual(const sp<IBase>& left, const sp<IBase>& right) {
    if (left == nullptr || right == nullptr || !left->isRemote() || !right->isRemote()) {
        return left == right;
    }
    return getOrCreateCachedBinder(left.get()) == getOrCreateCachedBinder(right.get());
}

namespace details {
int32_t getPidIfSharable() {
    return getpid();
}
}  // namespace details

}  // namespace hardware
}  // namespace android
