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

// All static variables go here, to control initialization and
// destruction order in the library.

#include <hidl/Static.h>
#include "InternalStatic.h"

#include <android/hidl/manager/1.0/IServiceManager.h>
#include <utils/Mutex.h>

namespace android {
namespace hardware {
namespace details {

// Deprecated; kept for ABI compatibility. Use getBnConstructorMap.
DoNotDestruct<BnConstructorMap> gBnConstructorMap{};

DoNotDestruct<ConcurrentMap<const ::android::hidl::base::V1_0::IBase*,
                            wp<::android::hardware::BHwBinder>>>
        gBnMap{};

// TODO(b/122472540): replace with single, hidden map
DoNotDestruct<ConcurrentMap<wp<::android::hidl::base::V1_0::IBase>, SchedPrio>> gServicePrioMap{};
DoNotDestruct<ConcurrentMap<wp<::android::hidl::base::V1_0::IBase>, bool>> gServiceSidMap{};

// Deprecated; kept for ABI compatibility. Use getBsConstructorMap.
DoNotDestruct<BsConstructorMap> gBsConstructorMap{};

// For static executables, it is not guaranteed that gBnConstructorMap are initialized before
// used in HAL definition libraries.
BnConstructorMap& getBnConstructorMap() {
    static BnConstructorMap& map = *new BnConstructorMap();
    return map;
}

BsConstructorMap& getBsConstructorMap() {
    static BsConstructorMap& map = *new BsConstructorMap();
    return map;
}

}  // namespace details
}  // namespace hardware
}  // namespace android
