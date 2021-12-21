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

#ifndef ANDROID_HARDWARE_ISERVICE_MANAGER_H
#define ANDROID_HARDWARE_ISERVICE_MANAGER_H

#include <string>
#include <vector>

#include <android/hidl/base/1.0/IBase.h>
#include <utils/StrongPointer.h>

namespace android {

namespace hidl {
namespace manager {
namespace V1_0 {
    struct IServiceManager;
}  // namespace V1_0
namespace V1_1 {
struct IServiceManager;
}  // namespace V1_1
namespace V1_2 {
struct IServiceManager;
}  // namespace V1_2
}  // namespace manager
}  // namespace hidl

namespace hardware {

namespace details {

// Will not attempt to start a lazy HAL
// e.x.: android.hardware.foo@1.0::IFoo, default
void waitForHwService(const std::string &interface, const std::string &instanceName);

// Only works on userdebug/eng builds. This allows getService to bypass the
// VINTF manifest for testing only.
void setTrebleTestingOverride(bool testingOverride);

void preloadPassthroughService(const std::string &descriptor);

// Returns a service with the following constraints:
// - retry => service is waited for and returned if it is declared in the
//     manifest AND it is available in this process (if errors indicate an
//     sepolicy denial, then this will return - TODO(b/28321379) more precise
//     errors to handle more cases)
// - getStub => internal only. Forces to get the unwrapped (no BsFoo) if available.
// TODO(b/65843592)
// If the service is a remote service, this function returns BpBase. If the service is
// a passthrough service, this function returns the appropriately wrapped Bs child object.
sp<::android::hidl::base::V1_0::IBase> getRawServiceInternal(const std::string& descriptor,
                                                             const std::string& instance,
                                                             bool retry, bool getStub);

status_t registerAsServiceInternal(const sp<::android::hidl::base::V1_0::IBase>& service,
                                   const std::string& name);
}  // namespace details

// These functions are for internal use by hidl. If you want to get ahold
// of an interface, the best way to do this is by calling IFoo::getService()
sp<::android::hidl::manager::V1_0::IServiceManager> defaultServiceManager();
sp<::android::hidl::manager::V1_1::IServiceManager> defaultServiceManager1_1();
sp<::android::hidl::manager::V1_2::IServiceManager> defaultServiceManager1_2();
sp<::android::hidl::manager::V1_0::IServiceManager> getPassthroughServiceManager();
sp<::android::hidl::manager::V1_1::IServiceManager> getPassthroughServiceManager1_1();

/**
 * Given a descriptor (e.g. from IFoo::descriptor), return a list of all instance names
 * on a device (e.g. the VINTF manifest). These HALs may not be currently running, but
 * the expectation is that if they aren't running, they should start as lazy HALs.
 * So, getService should return for each of these instance names.
 */
std::vector<std::string> getAllHalInstanceNames(const std::string& descriptor);

/**
 * Given a service that is in passthrough mode, this function will go ahead and load the
 * required passthrough module library (but not call HIDL_FETCH_I* functions to instantiate it).
 *
 * E.x.: preloadPassthroughService<IFoo>();
 */
template<typename I>
static inline void preloadPassthroughService() {
    details::preloadPassthroughService(I::descriptor);
}

} // namespace hardware
} // namespace android

#endif // ANDROID_HARDWARE_ISERVICE_MANAGER_H

