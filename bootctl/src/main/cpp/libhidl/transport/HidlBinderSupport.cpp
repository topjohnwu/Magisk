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

#define LOG_TAG "HidlSupport"

#include <hidl/HidlBinderSupport.h>

#include <android/hidl/base/1.0/BpHwBase.h>
#include <android/hidl/manager/1.0/BpHwServiceManager.h>
#include <android/hidl/manager/1.1/BpHwServiceManager.h>
#include <android/hidl/manager/1.2/BpHwServiceManager.h>
#include <hwbinder/IPCThreadState.h>
#include "InternalStatic.h"  // TODO(b/69122224): remove this include, for getOrCreateCachedBinder

// C includes
#include <inttypes.h>
#include <unistd.h>

// C++ includes
#include <fstream>
#include <sstream>

namespace android {
namespace hardware {

hidl_binder_death_recipient::hidl_binder_death_recipient(const sp<hidl_death_recipient> &recipient,
        uint64_t cookie, const sp<::android::hidl::base::V1_0::IBase> &base) :
    mRecipient(recipient), mCookie(cookie), mBase(base) {
}

void hidl_binder_death_recipient::binderDied(const wp<IBinder>& /*who*/) {
    sp<hidl_death_recipient> recipient = mRecipient.promote();
    if (recipient != nullptr && mBase != nullptr) {
        recipient->serviceDied(mCookie, mBase);
    }
    mBase = nullptr;
}

wp<hidl_death_recipient> hidl_binder_death_recipient::getRecipient() {
    return mRecipient;
}

const size_t hidl_handle::kOffsetOfNativeHandle = offsetof(hidl_handle, mHandle);
static_assert(hidl_handle::kOffsetOfNativeHandle == 0, "wrong offset");

status_t readEmbeddedFromParcel(const hidl_handle& /* handle */,
        const Parcel &parcel, size_t parentHandle, size_t parentOffset) {
    const native_handle_t *handle;
    status_t _hidl_err = parcel.readNullableEmbeddedNativeHandle(
            parentHandle,
            parentOffset + hidl_handle::kOffsetOfNativeHandle,
            &handle);

    return _hidl_err;
}

status_t writeEmbeddedToParcel(const hidl_handle &handle,
        Parcel *parcel, size_t parentHandle, size_t parentOffset) {
    status_t _hidl_err = parcel->writeEmbeddedNativeHandle(
            handle.getNativeHandle(),
            parentHandle,
            parentOffset + hidl_handle::kOffsetOfNativeHandle);

    return _hidl_err;
}

const size_t hidl_memory::kOffsetOfHandle = offsetof(hidl_memory, mHandle);
const size_t hidl_memory::kOffsetOfName = offsetof(hidl_memory, mName);
static_assert(hidl_memory::kOffsetOfHandle == 0, "wrong offset");
static_assert(hidl_memory::kOffsetOfName == 24, "wrong offset");

status_t readEmbeddedFromParcel(const hidl_memory& memory,
        const Parcel &parcel, size_t parentHandle, size_t parentOffset) {
    // TODO(b/111883309): Invoke readEmbeddedFromParcel(hidl_handle, ...).
    const native_handle_t *handle;
    ::android::status_t _hidl_err = parcel.readNullableEmbeddedNativeHandle(
            parentHandle,
            parentOffset + hidl_memory::kOffsetOfHandle,
            &handle);

    if (_hidl_err == ::android::OK) {
        _hidl_err = readEmbeddedFromParcel(
                memory.name(),
                parcel,
                parentHandle,
                parentOffset + hidl_memory::kOffsetOfName);
    }

    // hidl_memory's size is stored in uint64_t, but mapMemory's mmap will map
    // size in size_t. If size is over SIZE_MAX, mapMemory could succeed
    // but the mapped memory's actual size will be smaller than the reported size.
    if (memory.size() > SIZE_MAX) {
        ALOGE("Cannot use memory with %" PRId64 " bytes because it is too large.", memory.size());
        android_errorWriteLog(0x534e4554, "79376389");
        return BAD_VALUE;
    }

    return _hidl_err;
}

status_t writeEmbeddedToParcel(const hidl_memory &memory,
        Parcel *parcel, size_t parentHandle, size_t parentOffset) {
    // TODO(b/111883309): Invoke writeEmbeddedToParcel(hidl_handle, ...).
    status_t _hidl_err = parcel->writeEmbeddedNativeHandle(
            memory.handle(),
            parentHandle,
            parentOffset + hidl_memory::kOffsetOfHandle);

    if (_hidl_err == ::android::OK) {
        _hidl_err = writeEmbeddedToParcel(
            memory.name(),
            parcel,
            parentHandle,
            parentOffset + hidl_memory::kOffsetOfName);
    }

    return _hidl_err;
}
const size_t hidl_string::kOffsetOfBuffer = offsetof(hidl_string, mBuffer);
static_assert(hidl_string::kOffsetOfBuffer == 0, "wrong offset");

status_t readEmbeddedFromParcel(const hidl_string &string ,
        const Parcel &parcel, size_t parentHandle, size_t parentOffset) {
    const void *out;

    status_t status = parcel.readEmbeddedBuffer(
            string.size() + 1,
            nullptr /* buffer_handle */,
            parentHandle,
            parentOffset + hidl_string::kOffsetOfBuffer,
            &out);

    if (status != OK) {
        return status;
    }

    // Always safe to access out[string.size()] because we read size+1 bytes
    if (static_cast<const char *>(out)[string.size()] != '\0') {
        ALOGE("Received unterminated hidl_string buffer.");
        return BAD_VALUE;
    }

    return OK;
}

status_t writeEmbeddedToParcel(const hidl_string &string,
        Parcel *parcel, size_t parentHandle, size_t parentOffset) {
    return parcel->writeEmbeddedBuffer(
            string.c_str(),
            string.size() + 1,
            nullptr /* handle */,
            parentHandle,
            parentOffset + hidl_string::kOffsetOfBuffer);
}

status_t readFromParcel(Status *s, const Parcel& parcel) {
    int32_t exception;
    status_t status = parcel.readInt32(&exception);
    if (status != OK) {
        s->setFromStatusT(status);
        return status;
    }

    if (exception == Status::EX_NONE) {
        *s = Status::ok();
        return status;
    }

    // The remote threw an exception.  Get the message back.
    String16 message;
    status = parcel.readString16(&message);
    if (status != OK) {
        s->setFromStatusT(status);
        return status;
    }

    s->setException(exception, String8(message));

    return status;
}

status_t writeToParcel(const Status &s, Parcel* parcel) {
    // Something really bad has happened, and we're not going to even
    // try returning rich error data.
    if (s.exceptionCode() == Status::EX_TRANSACTION_FAILED) {
        return s.transactionError();
    }

    status_t status = parcel->writeInt32(s.exceptionCode());
    if (status != OK) { return status; }
    if (s.exceptionCode() == Status::EX_NONE) {
        // We have no more information to write.
        return status;
    }
    status = parcel->writeString16(String16(s.exceptionMessage()));
    return status;
}

// assume: iface != nullptr, iface isRemote
// This function is to sandbox a cast through a BpHw* class into a function, so
// that we can remove cfi sanitization from it. Do not add additional
// functionality here.
__attribute__((no_sanitize("cfi"))) static inline BpHwRefBase* forceGetRefBase(
        ::android::hidl::base::V1_0::IBase* ifacePtr) {
    using ::android::hidl::base::V1_0::BpHwBase;

    // canary only
    static_assert(sizeof(BpHwBase) == sizeof(hidl::manager::V1_0::BpHwServiceManager));
    static_assert(sizeof(BpHwBase) == sizeof(hidl::manager::V1_1::BpHwServiceManager));
    static_assert(sizeof(BpHwBase) == sizeof(hidl::manager::V1_2::BpHwServiceManager));

    // All BpHw* are generated the same. This may be BpHwServiceManager,
    // BpHwFoo, or any other class. For ABI compatibility, we can't modify the
    // class hierarchy of these, so we have no way to get BpHwRefBase from a
    // remote ifacePtr.
    BpHwBase* bpBase = static_cast<BpHwBase*>(ifacePtr);
    return static_cast<BpHwRefBase*>(bpBase);
}

sp<IBinder> getOrCreateCachedBinder(::android::hidl::base::V1_0::IBase* ifacePtr) {
    if (ifacePtr == nullptr) {
        return nullptr;
    }

    if (ifacePtr->isRemote()) {
        BpHwRefBase* bpRefBase = forceGetRefBase(ifacePtr);
        return sp<IBinder>(bpRefBase->remote());
    }

    std::string descriptor = details::getDescriptor(ifacePtr);
    if (descriptor.empty()) {
        // interfaceDescriptor fails
        return nullptr;
    }

    // for get + set
    std::unique_lock<std::mutex> _lock = details::gBnMap->lock();

    wp<BHwBinder> wBnObj = details::gBnMap->getLocked(ifacePtr, nullptr);
    sp<IBinder> sBnObj = wBnObj.promote();

    if (sBnObj == nullptr) {
        auto func = details::getBnConstructorMap().get(descriptor, nullptr);
        if (!func) {
            // TODO(b/69122224): remove this static variable when prebuilts updated
            func = details::gBnConstructorMap->get(descriptor, nullptr);
        }
        LOG_ALWAYS_FATAL_IF(func == nullptr, "%s gBnConstructorMap returned null for %s", __func__,
                            descriptor.c_str());

        sBnObj = sp<IBinder>(func(static_cast<void*>(ifacePtr)));
        LOG_ALWAYS_FATAL_IF(sBnObj == nullptr, "%s Bn constructor function returned null for %s",
                            __func__, descriptor.c_str());

        details::gBnMap->setLocked(ifacePtr, static_cast<BHwBinder*>(sBnObj.get()));
    }

    return sBnObj;
}

static bool gThreadPoolConfigured = false;

void configureBinderRpcThreadpool(size_t maxThreads, bool callerWillJoin) {
    status_t ret = ProcessState::self()->setThreadPoolConfiguration(
        maxThreads, callerWillJoin /*callerJoinsPool*/);
    LOG_ALWAYS_FATAL_IF(ret != OK, "Could not setThreadPoolConfiguration: %d", ret);

    gThreadPoolConfigured = true;
}

void joinBinderRpcThreadpool() {
    LOG_ALWAYS_FATAL_IF(!gThreadPoolConfigured,
                        "HIDL joinRpcThreadpool without calling configureRpcThreadPool.");
    IPCThreadState::self()->joinThreadPool();
}

int setupBinderPolling() {
    int fd;
    int err = IPCThreadState::self()->setupPolling(&fd);

    LOG_ALWAYS_FATAL_IF(err != OK, "Failed to setup binder polling: %d (%s)", err, strerror(err));

    return err == OK ? fd : -1;
}

status_t handleBinderPoll() {
    return IPCThreadState::self()->handlePolledCommands();
}

void addPostCommandTask(const std::function<void(void)> task) {
    IPCThreadState::self()->addPostCommandTask(task);
}

}  // namespace hardware
}  // namespace android
