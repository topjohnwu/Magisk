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

#ifndef _FMSGQ_DESCRIPTOR_H
#define _FMSGQ_DESCRIPTOR_H

#include <unistd.h>

#include <cutils/native_handle.h>
#include <fmq/MQDescriptorBase.h>
#include <hidl/HidlInternal.h>
#include <hidl/HidlSupport.h>

namespace android {
namespace hardware {

template <typename T, MQFlavor flavor>
struct MQDescriptor {
    // Takes ownership of handle
    MQDescriptor(
            const std::vector<GrantorDescriptor>& grantors,
            native_handle_t* nHandle, size_t size);

    // Takes ownership of handle
    MQDescriptor(size_t bufferSize, native_handle_t* nHandle,
                 size_t messageSize, bool configureEventFlag = false);

    MQDescriptor();
    ~MQDescriptor();

    explicit MQDescriptor(const MQDescriptor& other) : MQDescriptor() { *this = other; }
    MQDescriptor& operator=(const MQDescriptor& other);

    size_t getSize() const;

    size_t getQuantum() const;

    int32_t getFlags() const;

    bool isHandleValid() const { return mHandle != nullptr; }
    size_t countGrantors() const { return mGrantors.size(); }

    inline const ::android::hardware::hidl_vec<GrantorDescriptor> &grantors() const {
        return mGrantors;
    }

    inline const ::native_handle_t *handle() const {
        return mHandle;
    }

    inline ::native_handle_t *handle() {
        return mHandle;
    }

    static const size_t kOffsetOfGrantors;
    static const size_t kOffsetOfHandle;

private:
    ::android::hardware::hidl_vec<GrantorDescriptor> mGrantors;
    ::android::hardware::details::hidl_pointer<native_handle_t> mHandle;
    uint32_t mQuantum;
    uint32_t mFlags;
};

template<typename T, MQFlavor flavor>
const size_t MQDescriptor<T, flavor>::kOffsetOfGrantors = offsetof(MQDescriptor, mGrantors);

template<typename T, MQFlavor flavor>
const size_t MQDescriptor<T, flavor>::kOffsetOfHandle = offsetof(MQDescriptor, mHandle);

/*
 * MQDescriptorSync will describe the wait-free synchronized
 * flavor of FMQ.
 */
template<typename T>
using MQDescriptorSync = MQDescriptor<T, kSynchronizedReadWrite>;

/*
 * MQDescriptorUnsync will describe the unsynchronized write
 * flavor of FMQ.
 */
template<typename T>
using MQDescriptorUnsync = MQDescriptor<T, kUnsynchronizedWrite>;

template <typename T, MQFlavor flavor>
MQDescriptor<T, flavor>::MQDescriptor(const std::vector<GrantorDescriptor>& grantors,
                                      native_handle_t* nhandle, size_t size)
    : mHandle(nhandle), mQuantum(static_cast<uint32_t>(size)), mFlags(flavor) {
    mGrantors.resize(grantors.size());
    for (size_t i = 0; i < grantors.size(); ++i) {
        mGrantors[i] = grantors[i];
    }
}

template <typename T, MQFlavor flavor>
MQDescriptor<T, flavor>::MQDescriptor(size_t bufferSize, native_handle_t* nHandle,
                                      size_t messageSize, bool configureEventFlag)
    : mHandle(nHandle), mQuantum(static_cast<uint32_t>(messageSize)), mFlags(flavor) {
    /*
     * If configureEventFlag is true, allocate an additional spot in mGrantor
     * for containing the fd and offset for mmapping the EventFlag word.
     */
    mGrantors.resize(configureEventFlag ? details::kMinGrantorCountForEvFlagSupport
                                        : details::kMinGrantorCount);

    size_t memSize[] = {
            sizeof(details::RingBufferPosition), /* memory to be allocated for read pointer counter
                                                  */
            sizeof(details::RingBufferPosition), /* memory to be allocated for write pointer counter
                                                  */
            bufferSize,                          /* memory to be allocated for data buffer */
            sizeof(std::atomic<uint32_t>)        /* memory to be allocated for EventFlag word */
    };

    /*
     * Create a default grantor descriptor for read, write pointers and
     * the data buffer. fdIndex parameter is set to 0 by default and
     * the offset for each grantor is contiguous.
     */
    for (size_t grantorPos = 0, offset = 0;
         grantorPos < mGrantors.size();
         offset += memSize[grantorPos++]) {
        mGrantors[grantorPos] = {0 /* grantor flags */, 0 /* fdIndex */,
                                 static_cast<uint32_t>(details::alignToWordBoundary(offset)),
                                 memSize[grantorPos]};
    }
}

template <typename T, MQFlavor flavor>
MQDescriptor<T, flavor>& MQDescriptor<T, flavor>::operator=(const MQDescriptor& other) {
    mGrantors = other.mGrantors;
    if (mHandle != nullptr) {
        native_handle_close(mHandle);
        native_handle_delete(mHandle);
        mHandle = nullptr;
    }
    mQuantum = other.mQuantum;
    mFlags = other.mFlags;

    if (other.mHandle != nullptr) {
        mHandle = native_handle_create(
                other.mHandle->numFds, other.mHandle->numInts);

        for (int i = 0; i < other.mHandle->numFds; ++i) {
            mHandle->data[i] = dup(other.mHandle->data[i]);
        }

        memcpy(&mHandle->data[other.mHandle->numFds], &other.mHandle->data[other.mHandle->numFds],
               static_cast<size_t>(other.mHandle->numInts) * sizeof(int));
    }

    return *this;
}

template<typename T, MQFlavor flavor>
MQDescriptor<T, flavor>::MQDescriptor() : MQDescriptor(
        std::vector<android::hardware::GrantorDescriptor>(),
        nullptr /* nHandle */,
        0 /* size */) {}

template<typename T, MQFlavor flavor>
MQDescriptor<T, flavor>::~MQDescriptor() {
    if (mHandle != nullptr) {
        native_handle_close(mHandle);
        native_handle_delete(mHandle);
    }
}

template<typename T, MQFlavor flavor>
size_t MQDescriptor<T, flavor>::getSize() const {
    return static_cast<size_t>(mGrantors[details::DATAPTRPOS].extent);
}

template<typename T, MQFlavor flavor>
size_t MQDescriptor<T, flavor>::getQuantum() const { return mQuantum; }

template<typename T, MQFlavor flavor>
int32_t MQDescriptor<T, flavor>::getFlags() const { return mFlags; }

template<typename T, MQFlavor flavor>
std::string toString(const MQDescriptor<T, flavor> &q) {
    std::string os;
    if (flavor & kSynchronizedReadWrite) {
        os += "fmq_sync";
    }
    if (flavor & kUnsynchronizedWrite) {
        os += "fmq_unsync";
    }
    os += " {"
       + toString(q.grantors().size()) + " grantor(s), "
       + "size = " + toString(q.getSize())
       + ", .handle = " + toString(q.handle())
       + ", .quantum = " + toString(q.getQuantum()) + "}";
    return os;
}

}  // namespace hardware
}  // namespace android

#endif  // FMSGQ_DESCRIPTOR_H
