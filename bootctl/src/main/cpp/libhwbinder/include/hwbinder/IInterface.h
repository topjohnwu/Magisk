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

//
#ifndef ANDROID_HARDWARE_IINTERFACE_H
#define ANDROID_HARDWARE_IINTERFACE_H

#include <hwbinder/Binder.h>

// WARNING: this code is part of libhwbinder, a fork of libbinder. Generally,
// this means that it is only relevant to HIDL. Any AIDL- or libbinder-specific
// code should not try to use these things.

namespace android {
namespace hardware {
// ----------------------------------------------------------------------

class IInterface : public virtual RefBase
{
public:
            IInterface();
            static sp<IBinder>  asBinder(const IInterface*);
            static sp<IBinder>  asBinder(const sp<IInterface>&);
protected:
    virtual                     ~IInterface();
    virtual IBinder*            onAsBinder() = 0;
};

// ----------------------------------------------------------------------

template<typename INTERFACE>
class BpInterface : public INTERFACE, public IInterface, public BpHwRefBase
{
public:
    explicit                    BpInterface(const sp<IBinder>& remote);
    virtual IBinder*            onAsBinder();
};

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// No user-serviceable parts after this...


template<typename INTERFACE>
inline BpInterface<INTERFACE>::BpInterface(const sp<IBinder>& remote)
    : BpHwRefBase(remote)
{
}

template<typename INTERFACE>
inline IBinder* BpInterface<INTERFACE>::onAsBinder()
{
    return remote();
}

// ----------------------------------------------------------------------

} // namespace hardware
} // namespace android

#endif // ANDROID_HARDWARE_IINTERFACE_H
