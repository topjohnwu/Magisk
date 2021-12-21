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

#ifndef ANDROID_HIDL_BINDER_SUPPORT_H
#define ANDROID_HIDL_BINDER_SUPPORT_H

#include <sys/types.h>

#include <android/hidl/base/1.0/BnHwBase.h>
#include <android/hidl/base/1.0/IBase.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportUtils.h>
#include <hidl/MQDescriptor.h>
#include <hwbinder/IBinder.h>
#include <hwbinder/Parcel.h>
#include <log/log.h>  // TODO(b/65843592): remove. Too many users depending on this transitively.

// Defines functions for hidl_string, Status, hidl_vec, MQDescriptor,
// etc. to interact with Parcel.

namespace android {
namespace hardware {

// hidl_binder_death_recipient wraps a transport-independent
// hidl_death_recipient, and implements the binder-specific
// DeathRecipient interface.
struct hidl_binder_death_recipient : IBinder::DeathRecipient {
    hidl_binder_death_recipient(const sp<hidl_death_recipient> &recipient,
            uint64_t cookie, const sp<::android::hidl::base::V1_0::IBase> &base);
    virtual void binderDied(const wp<IBinder>& /*who*/);
    wp<hidl_death_recipient> getRecipient();
private:
    wp<hidl_death_recipient> mRecipient;
    uint64_t mCookie;
    wp<::android::hidl::base::V1_0::IBase> mBase;
};

// ---------------------- hidl_handle

status_t readEmbeddedFromParcel(const hidl_handle &handle,
        const Parcel &parcel, size_t parentHandle, size_t parentOffset);

status_t writeEmbeddedToParcel(const hidl_handle &handle,
        Parcel *parcel, size_t parentHandle, size_t parentOffset);

// ---------------------- hidl_memory

status_t readEmbeddedFromParcel(const hidl_memory &memory,
        const Parcel &parcel, size_t parentHandle, size_t parentOffset);

status_t writeEmbeddedToParcel(const hidl_memory &memory,
        Parcel *parcel, size_t parentHandle, size_t parentOffset);

// ---------------------- hidl_string

status_t readEmbeddedFromParcel(const hidl_string &string,
        const Parcel &parcel, size_t parentHandle, size_t parentOffset);

status_t writeEmbeddedToParcel(const hidl_string &string,
        Parcel *parcel, size_t parentHandle, size_t parentOffset);

// ---------------------- Status

// Bear in mind that if the client or service is a Java endpoint, this
// is not the logic which will provide/interpret the data here.
status_t readFromParcel(Status *status, const Parcel& parcel);
status_t writeToParcel(const Status &status, Parcel* parcel);

// ---------------------- hidl_vec

template<typename T>
status_t readEmbeddedFromParcel(
        const hidl_vec<T> &vec,
        const Parcel &parcel,
        size_t parentHandle,
        size_t parentOffset,
        size_t *handle) {
    const void *out;
    return parcel.readNullableEmbeddedBuffer(
            vec.size() * sizeof(T),
            handle,
            parentHandle,
            parentOffset + hidl_vec<T>::kOffsetOfBuffer,
            &out);
}

template<typename T>
status_t writeEmbeddedToParcel(
        const hidl_vec<T> &vec,
        Parcel *parcel,
        size_t parentHandle,
        size_t parentOffset,
        size_t *handle) {
    return parcel->writeEmbeddedBuffer(
            vec.data(),
            sizeof(T) * vec.size(),
            handle,
            parentHandle,
            parentOffset + hidl_vec<T>::kOffsetOfBuffer);
}

template<typename T>
status_t findInParcel(const hidl_vec<T> &vec, const Parcel &parcel, size_t *handle) {
    return parcel.quickFindBuffer(vec.data(), handle);
}

// ---------------------- MQDescriptor

template<typename T, MQFlavor flavor>
::android::status_t readEmbeddedFromParcel(
        MQDescriptor<T, flavor> &obj,
        const ::android::hardware::Parcel &parcel,
        size_t parentHandle,
        size_t parentOffset) {
    ::android::status_t _hidl_err = ::android::OK;

    size_t _hidl_grantors_child;

    _hidl_err = ::android::hardware::readEmbeddedFromParcel(
                obj.grantors(),
                parcel,
                parentHandle,
                parentOffset + MQDescriptor<T, flavor>::kOffsetOfGrantors,
                &_hidl_grantors_child);

    if (_hidl_err != ::android::OK) { return _hidl_err; }

    const native_handle_t *_hidl_mq_handle_ptr;
   _hidl_err = parcel.readNullableEmbeddedNativeHandle(
            parentHandle,
            parentOffset + MQDescriptor<T, flavor>::kOffsetOfHandle,
            &_hidl_mq_handle_ptr);

    if (_hidl_err != ::android::OK) { return _hidl_err; }

    return _hidl_err;
}

template<typename T, MQFlavor flavor>
::android::status_t writeEmbeddedToParcel(
        const MQDescriptor<T, flavor> &obj,
        ::android::hardware::Parcel *parcel,
        size_t parentHandle,
        size_t parentOffset) {
    ::android::status_t _hidl_err = ::android::OK;

    size_t _hidl_grantors_child;

    _hidl_err = ::android::hardware::writeEmbeddedToParcel(
            obj.grantors(),
            parcel,
            parentHandle,
            parentOffset + MQDescriptor<T, flavor>::kOffsetOfGrantors,
            &_hidl_grantors_child);

    if (_hidl_err != ::android::OK) { return _hidl_err; }

    _hidl_err = parcel->writeEmbeddedNativeHandle(
            obj.handle(),
            parentHandle,
            parentOffset + MQDescriptor<T, flavor>::kOffsetOfHandle);

    if (_hidl_err != ::android::OK) { return _hidl_err; }

    return _hidl_err;
}

// ---------------------- support for casting interfaces

// Constructs a binder for this interface and caches it. If it has already been created
// then it returns it.
sp<IBinder> getOrCreateCachedBinder(::android::hidl::base::V1_0::IBase* ifacePtr);

// Construct a smallest possible binder from the given interface.
// If it is remote, then its remote() will be retrieved.
// Otherwise, the smallest possible BnChild is found where IChild is a subclass of IType
// and iface is of class IChild. BnChild will be used to wrapped the given iface.
// Return nullptr if iface is null or any failure.
template <typename IType,
          typename = std::enable_if_t<std::is_same<details::i_tag, typename IType::_hidl_tag>::value>>
sp<IBinder> toBinder(sp<IType> iface) {
    IType *ifacePtr = iface.get();
    return getOrCreateCachedBinder(ifacePtr);
}

template <typename IType, typename ProxyType, typename StubType>
sp<IType> fromBinder(const sp<IBinder>& binderIface) {
    using ::android::hidl::base::V1_0::IBase;
    using ::android::hidl::base::V1_0::BnHwBase;

    if (binderIface.get() == nullptr) {
        return nullptr;
    }

    if (binderIface->localBinder() == nullptr) {
        return new ProxyType(binderIface);
    }

    // Ensure that IBinder is BnHwBase (not JHwBinder, for instance)
    if (!binderIface->checkSubclass(IBase::descriptor)) {
        return new ProxyType(binderIface);
    }
    sp<IBase> base = static_cast<BnHwBase*>(binderIface.get())->getImpl();

    if (details::canCastInterface(base.get(), IType::descriptor)) {
        return static_cast<IType*>(base.get());
    } else {
        return nullptr;
    }
}

void configureBinderRpcThreadpool(size_t maxThreads, bool callerWillJoin);
void joinBinderRpcThreadpool();
int setupBinderPolling();
status_t handleBinderPoll();

void addPostCommandTask(const std::function<void(void)> task);

}  // namespace hardware
}  // namespace android


#endif  // ANDROID_HIDL_BINDER_SUPPORT_H
