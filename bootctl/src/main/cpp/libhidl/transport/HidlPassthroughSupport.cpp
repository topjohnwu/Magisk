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

#include <hidl/HidlPassthroughSupport.h>

#include "InternalStatic.h"  // TODO(b/69122224): remove this include, for tryWrap

#include <hidl/HidlTransportUtils.h>
#include <hidl/Static.h>

using ::android::hidl::base::V1_0::IBase;

namespace android {
namespace hardware {
namespace details {

static sp<IBase> tryWrap(const std::string& descriptor, sp<IBase> iface) {
    auto func = getBsConstructorMap().get(descriptor, nullptr);
    if (!func) {
        // TODO(b/69122224): remove this when prebuilts don't reference it
        func = gBsConstructorMap->get(descriptor, nullptr);
    }
    if (func) {
        return func(static_cast<void*>(iface.get()));
    }
    return nullptr;
}

sp<IBase> wrapPassthroughInternal(sp<IBase> iface) {
    if (iface == nullptr || iface->isRemote()) {
        // doesn't know how to handle it.
        return iface;
    }

    // Consider the case when an AOSP interface is extended by partners.
    // Then the partner's HAL interface library is loaded only in the vndk
    // linker namespace, but not in the default linker namespace, where
    // this code runs. As a result, BsConstructorMap in the latter does not
    // have the entry for the descriptor name.
    //
    // Therefore, we try to wrap using the descript names of the parent
    // types along the interface chain, instead of always using the descriptor
    // name of the current interface.
    sp<IBase> base;
    auto ret = iface->interfaceChain([&](const auto& types) {
        for (const std::string& descriptor : types) {
            base = tryWrap(descriptor, iface);
            if (base != nullptr) {
                break;  // wrap is successful. no need to lookup further.
            }
        }
    });

    if (!ret.isOk()) {
        return nullptr;
    }

    // It is ensured that if this function is called with an instance of IType
    // then the corresponding descriptor would be in the BsConstructorMap.
    // This is because referencing IType implies that the interface library
    // defining the type has already been loaded into the current linker
    // namespace, and thus the library should have added an entry into the
    // BsConstructorMap while executing the library's constructor.
    return base;
}

}  // namespace details
}  // namespace hardware
}  // namespace android
