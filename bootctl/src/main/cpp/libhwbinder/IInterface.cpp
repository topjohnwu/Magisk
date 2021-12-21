/*
 * Copyright (C) 2005 The Android Open Source Project
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

#define LOG_TAG "hw-IInterface"
#include <utils/Log.h>
#include <hwbinder/IInterface.h>

namespace android {
namespace hardware {

// ---------------------------------------------------------------------------

IInterface::IInterface() 
    : RefBase() {
}

IInterface::~IInterface() {
}

// static
sp<IBinder> IInterface::asBinder(const IInterface* iface)
{
    if (iface == nullptr) return nullptr;
    return const_cast<IInterface*>(iface)->onAsBinder();
}

// static
sp<IBinder> IInterface::asBinder(const sp<IInterface>& iface)
{
    if (iface == nullptr) return nullptr;
    return iface->onAsBinder();
}


// ---------------------------------------------------------------------------

} // namespace hardware
} // namespace android
