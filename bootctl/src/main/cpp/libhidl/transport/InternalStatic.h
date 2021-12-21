/*
 * Copyright (C) 2018 The Android Open Source Project
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

// This file is for legacy static variables that we are trying to get rid of.
// TODO(b/69122224): remove this file

#ifndef ANDROID_HARDWARE_HIDL_INTERNAL_STATIC_H
#define ANDROID_HARDWARE_HIDL_INTERNAL_STATIC_H

#include <hidl/HidlTransportSupport.h>  // for SchedPrio
#include <hidl/Static.h>

namespace android {
namespace hardware {
namespace details {

// TODO(b/69122224): remove this once no prebuilts reference it
// deprecated; use getBnConstructorMap instead.
extern DoNotDestruct<BnConstructorMap> gBnConstructorMap;
// TODO(b/69122224): remove this once no prebuilts reference it
// deprecated; use getBsConstructorMap instead.
extern DoNotDestruct<BsConstructorMap> gBsConstructorMap;

// TODO(b/69122224): remove this once no prebuilts reference it
extern DoNotDestruct<ConcurrentMap<wp<::android::hidl::base::V1_0::IBase>, SchedPrio>>
        gServicePrioMap;
// TODO(b/69122224): remove this once no prebuilts reference it
extern DoNotDestruct<ConcurrentMap<wp<::android::hidl::base::V1_0::IBase>, bool>> gServiceSidMap;

}  // namespace details
}  // namespace hardware
}  // namespace android

#endif
