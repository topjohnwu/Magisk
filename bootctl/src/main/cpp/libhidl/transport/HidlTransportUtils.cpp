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

#include <hidl/HidlTransportUtils.h>

#include <android/hidl/base/1.0/IBase.h>

namespace android {
namespace hardware {
namespace details {

using ::android::hidl::base::V1_0::IBase;

Return<bool> canCastInterface(IBase* interface, const char* castTo, bool emitError) {
    if (interface == nullptr) {
        return false;
    }

    // b/68217907
    // Every HIDL interface is a base interface.
    if (std::string(IBase::descriptor) == castTo) {
        return true;
    }

    bool canCast = false;
    auto chainRet = interface->interfaceChain([&](const hidl_vec<hidl_string> &types) {
        for (size_t i = 0; i < types.size(); i++) {
            if (types[i] == castTo) {
                canCast = true;
                break;
            }
        }
    });

    if (!chainRet.isOk()) {
        // call fails, propagate the error if emitError
        return emitError
                ? details::StatusOf<void, bool>(chainRet)
                : Return<bool>(false);
    }

    return canCast;
}

std::string getDescriptor(IBase* interface) {
    if (interface == nullptr) {
        return "";
    }

    std::string myDescriptor{};
    auto ret = interface->interfaceDescriptor([&](const hidl_string &types) {
        myDescriptor = types.c_str();
    });
    ret.isOk(); // ignored, return empty string if not isOk()
    return myDescriptor;
}

}  // namespace details
}  // namespace hardware
}  // namespace android
