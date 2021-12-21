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

#include <string>

#include <hidl/HidlLazyUtils.h>
#include <hidl/HidlTransportSupport.h>
#include <sys/wait.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/StrongPointer.h>

#pragma once

namespace android {
namespace hardware {
namespace details {

using RegisterServiceCb =
        std::function<status_t(const sp<::android::hidl::base::V1_0::IBase>&, const std::string&)>;

__attribute__((warn_unused_result)) status_t registerPassthroughServiceImplementation(
        const std::string& interfaceName, const std::string& expectInterfaceName,
        RegisterServiceCb registerServiceCb, const std::string& serviceName = "default");

}  // namespace details

/**
 * Registers passthrough service implementation.
 */
__attribute__((warn_unused_result)) status_t registerPassthroughServiceImplementation(
        const std::string& interfaceName, const std::string& expectInterfaceName,
        const std::string& serviceName);

inline __attribute__((warn_unused_result)) status_t registerPassthroughServiceImplementation(
        const std::string& interfaceName, const std::string& serviceName = "default") {
    return registerPassthroughServiceImplementation(interfaceName, interfaceName, serviceName);
}

template <class Interface, class ExpectInterface = Interface>
__attribute__((warn_unused_result)) status_t registerPassthroughServiceImplementation(
        const std::string& name = "default") {
    return registerPassthroughServiceImplementation(Interface::descriptor,
                                                    ExpectInterface::descriptor, name);
}

/**
 * Creates default passthrough service implementation. This method never returns.
 *
 * Return value is exit status.
 */
template <class Interface, class ExpectInterface = Interface>
__attribute__((warn_unused_result)) status_t defaultPassthroughServiceImplementation(
        const std::string& name, size_t maxThreads = 1) {
    configureRpcThreadpool(maxThreads, true);
    status_t result = registerPassthroughServiceImplementation<Interface, ExpectInterface>(name);

    if (result != OK) {
        return result;
    }

    joinRpcThreadpool();
    return UNKNOWN_ERROR;
}
template <class Interface, class ExpectInterface = Interface>
__attribute__((warn_unused_result)) status_t defaultPassthroughServiceImplementation(
        size_t maxThreads = 1) {
    return defaultPassthroughServiceImplementation<Interface, ExpectInterface>("default",
                                                                               maxThreads);
}

/**
 * Registers a passthrough service implementation that exits when there are 0 clients.
 *
 * If this function is called multiple times to register different services, then this process will
 * only exit once all services have 0 clients. This function does not know about clients registered
 * through registerPassthroughServiceImplementation, so if that function is used in conjunction with
 * this one, the process may exit while a client is still using the HAL.
 */
template <class Interface, class ExpectInterface = Interface>
__attribute__((warn_unused_result)) status_t registerLazyPassthroughServiceImplementation(
        const std::string& name = "default") {
    return details::registerPassthroughServiceImplementation(
            Interface::descriptor, ExpectInterface::descriptor,
            [](const sp<::android::hidl::base::V1_0::IBase>& service, const std::string& name) {
                using android::hardware::LazyServiceRegistrar;
                return LazyServiceRegistrar::getInstance().registerService(service, name);
            },
            name);
}

/**
 * Creates default passthrough service implementation that exits when there are 0 clients. This
 * method never returns.
 *
 * Return value is exit status.
 */
template <class Interface, class ExpectInterface = Interface>
__attribute__((warn_unused_result)) status_t defaultLazyPassthroughServiceImplementation(
        const std::string& name, size_t maxThreads = 1) {
    configureRpcThreadpool(maxThreads, true);
    status_t result =
            registerLazyPassthroughServiceImplementation<Interface, ExpectInterface>(name);

    if (result != OK) {
        return result;
    }

    joinRpcThreadpool();
    return UNKNOWN_ERROR;
}
template <class Interface, class ExpectInterface = Interface>
__attribute__((warn_unused_result)) status_t defaultLazyPassthroughServiceImplementation(
        size_t maxThreads = 1) {
    return defaultLazyPassthroughServiceImplementation<Interface, ExpectInterface>("default",
                                                                                   maxThreads);
}

}  // namespace hardware
}  // namespace android
