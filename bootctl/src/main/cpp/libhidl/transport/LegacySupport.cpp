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

#define LOG_TAG "LegacySupport"

#include <android/hidl/base/1.0/IBase.h>
#include <hidl/HidlSupport.h>
#include <hidl/LegacySupport.h>
#include <hidl/ServiceManagement.h>
#include <hidl/Status.h>

using android::hidl::base::V1_0::IBase;

namespace android::hardware {

namespace details {

__attribute__((warn_unused_result)) status_t registerPassthroughServiceImplementation(
        const std::string& interfaceName, const std::string& expectInterfaceName,
        RegisterServiceCb registerServiceCb, const std::string& serviceName) {
    sp<IBase> service =
            getRawServiceInternal(interfaceName, serviceName, true /*retry*/, true /*getStub*/);

    if (service == nullptr) {
        ALOGE("Could not get passthrough implementation for %s/%s.", interfaceName.c_str(),
              serviceName.c_str());
        return EXIT_FAILURE;
    }
    if (service->isRemote()) {
        ALOGE("Implementation of %s/%s is remote!", interfaceName.c_str(), serviceName.c_str());
        return EXIT_FAILURE;
    }

    std::string actualName;
    Return<void> result = service->interfaceDescriptor(
            [&actualName](const hidl_string& descriptor) { actualName = descriptor; });
    if (!result.isOk()) {
        ALOGE("Error retrieving interface name from %s/%s: %s", interfaceName.c_str(),
              serviceName.c_str(), result.description().c_str());
        return EXIT_FAILURE;
    }
    if (actualName != expectInterfaceName) {
        ALOGE("Implementation of %s/%s is actually %s, not a %s!", interfaceName.c_str(),
              serviceName.c_str(), actualName.c_str(), expectInterfaceName.c_str());
        return EXIT_FAILURE;
    }

    status_t status = registerServiceCb(service, serviceName);
    if (status == OK) {
        ALOGI("Registration complete for %s/%s.", interfaceName.c_str(), serviceName.c_str());
    } else {
        ALOGE("Could not register service %s/%s (%d).", interfaceName.c_str(), serviceName.c_str(),
              status);
    }

    return status;
}

}  // namespace details

__attribute__((warn_unused_result)) status_t registerPassthroughServiceImplementation(
        const std::string& interfaceName, const std::string& expectInterfaceName,
        const std::string& serviceName) {
    return details::registerPassthroughServiceImplementation(
            interfaceName, expectInterfaceName,
            [](const sp<IBase>& service, const std::string& name) {
                return details::registerAsServiceInternal(service, name);
            },
            serviceName);
}

}  // namespace android::hardware
