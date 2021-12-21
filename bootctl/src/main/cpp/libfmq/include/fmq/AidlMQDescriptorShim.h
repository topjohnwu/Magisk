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

#include <aidl/android/hardware/common/fmq/MQDescriptor.h>
#include <cutils/native_handle.h>
#include <fmq/MQDescriptorBase.h>
#include <limits>
#include <type_traits>

namespace android {
namespace details {

using aidl::android::hardware::common::fmq::GrantorDescriptor;
using aidl::android::hardware::common::fmq::MQDescriptor;
using aidl::android::hardware::common::fmq::SynchronizedReadWrite;
using aidl::android::hardware::common::fmq::UnsynchronizedWrite;
using android::hardware::MQFlavor;

template <typename T, MQFlavor flavor>
struct AidlMQDescriptorShim {
    // Takes ownership of handle
    AidlMQDescriptorShim(const std::vector<android::hardware::GrantorDescriptor>& grantors,
                         native_handle_t* nHandle, size_t size);

    // Takes ownership of handle
    AidlMQDescriptorShim(
            const MQDescriptor<
                    T, typename std::conditional<flavor == hardware::kSynchronizedReadWrite,
                                                 SynchronizedReadWrite, UnsynchronizedWrite>::type>&
                    desc);

    // Takes ownership of handle
    AidlMQDescriptorShim(size_t bufferSize, native_handle_t* nHandle, size_t messageSize,
                         bool configureEventFlag = false);

    explicit AidlMQDescriptorShim(const AidlMQDescriptorShim& other)
        : AidlMQDescriptorShim(0, nullptr, 0) {
        *this = other;
    }
    AidlMQDescriptorShim& operator=(const AidlMQDescriptorShim& other);

    ~AidlMQDescriptorShim();

    size_t getSize() const;

    size_t getQuantum() const;

    uint32_t getFlags() const;

    bool isHandleValid() const { return mHandle != nullptr; }
    size_t countGrantors() const { return mGrantors.size(); }

    inline const std::vector<android::hardware::GrantorDescriptor>& grantors() const {
        return mGrantors;
    }

    inline const ::native_handle_t* handle() const { return mHandle; }

    inline ::native_handle_t* handle() { return mHandle; }

    static const size_t kOffsetOfGrantors;
    static const size_t kOffsetOfHandle;

  private:
    std::vector<android::hardware::GrantorDescriptor> mGrantors;
    native_handle_t* mHandle = nullptr;
    uint32_t mQuantum = 0;
    uint32_t mFlags = 0;
};

template <typename T, MQFlavor flavor>
AidlMQDescriptorShim<T, flavor>::AidlMQDescriptorShim(
        const MQDescriptor<T, typename std::conditional<flavor == hardware::kSynchronizedReadWrite,
                                                        SynchronizedReadWrite,
                                                        UnsynchronizedWrite>::type>& desc)
    : mQuantum(desc.quantum), mFlags(desc.flags) {
    if (desc.quantum < 0 || desc.flags < 0) {
        // MQDescriptor uses signed integers, but the values must be positive.
        hardware::details::logError("Invalid MQDescriptor. Values must be positive. quantum: " +
                                    std::to_string(desc.quantum) +
                                    ". flags: " + std::to_string(desc.flags));
        return;
    }

    mGrantors.resize(desc.grantors.size());
    for (size_t i = 0; i < desc.grantors.size(); ++i) {
        if (desc.grantors[i].offset < 0 || desc.grantors[i].extent < 0 ||
            desc.grantors[i].fdIndex < 0) {
            // GrantorDescriptor uses signed integers, but the values must be positive.
            // Return before setting up the native_handle to make this invalid.
            hardware::details::logError(
                    "Invalid MQDescriptor grantors. Values must be positive. Grantor index: " +
                    std::to_string(i) + ". offset: " + std::to_string(desc.grantors[i].offset) +
                    ". extent: " + std::to_string(desc.grantors[i].extent));
            return;
        }
        mGrantors[i].flags = 0;
        mGrantors[i].fdIndex = desc.grantors[i].fdIndex;
        mGrantors[i].offset = desc.grantors[i].offset;
        mGrantors[i].extent = desc.grantors[i].extent;
    }

    mHandle = native_handle_create(desc.handle.fds.size() /* num fds */,
                                   desc.handle.ints.size() /* num ints */);
    if (mHandle == nullptr) {
        hardware::details::logError("Null native_handle_t");
        return;
    }
    int data_index = 0;
    for (const auto& fd : desc.handle.fds) {
        mHandle->data[data_index] = dup(fd.get());
        data_index++;
    }
    for (const auto& data_int : desc.handle.ints) {
        mHandle->data[data_index] = data_int;
        data_index++;
    }
}

template <typename T, MQFlavor flavor>
AidlMQDescriptorShim<T, flavor>::AidlMQDescriptorShim(
        const std::vector<android::hardware::GrantorDescriptor>& grantors, native_handle_t* nhandle,
        size_t size)
    : mGrantors(grantors),
      mHandle(nhandle),
      mQuantum(static_cast<uint32_t>(size)),
      mFlags(flavor) {}

template <typename T, MQFlavor flavor>
AidlMQDescriptorShim<T, flavor>& AidlMQDescriptorShim<T, flavor>::operator=(
        const AidlMQDescriptorShim& other) {
    mGrantors = other.mGrantors;
    if (mHandle != nullptr) {
        native_handle_close(mHandle);
        native_handle_delete(mHandle);
        mHandle = nullptr;
    }
    mQuantum = other.mQuantum;
    mFlags = other.mFlags;

    if (other.mHandle != nullptr) {
        mHandle = native_handle_create(other.mHandle->numFds, other.mHandle->numInts);

        for (int i = 0; i < other.mHandle->numFds; ++i) {
            mHandle->data[i] = dup(other.mHandle->data[i]);
        }

        memcpy(&mHandle->data[other.mHandle->numFds], &other.mHandle->data[other.mHandle->numFds],
               static_cast<size_t>(other.mHandle->numInts) * sizeof(int));
    }

    return *this;
}

template <typename T, MQFlavor flavor>
AidlMQDescriptorShim<T, flavor>::AidlMQDescriptorShim(size_t bufferSize, native_handle_t* nHandle,
                                                      size_t messageSize, bool configureEventFlag)
    : mHandle(nHandle), mQuantum(messageSize), mFlags(flavor) {
    /*
     * TODO(b/165674950) Since AIDL does not support unsigned integers, it can only support
     * The offset of EventFlag word needs to fit into an int32_t in MQDescriptor. This word comes
     * after the readPtr, writePtr, and dataBuffer.
     */
    bool overflow = bufferSize > std::numeric_limits<uint64_t>::max() -
                                         (sizeof(hardware::details::RingBufferPosition) +
                                          sizeof(hardware::details::RingBufferPosition));
    uint64_t largestOffset = hardware::details::alignToWordBoundary(
            sizeof(hardware::details::RingBufferPosition) +
            sizeof(hardware::details::RingBufferPosition) + bufferSize);
    if (overflow || largestOffset > std::numeric_limits<int32_t>::max() ||
        messageSize > std::numeric_limits<int32_t>::max()) {
        hardware::details::logError(
                "Queue size is too large. Message size: " + std::to_string(messageSize) +
                " bytes. Data buffer size: " + std::to_string(bufferSize) + " bytes. Max size: " +
                std::to_string(std::numeric_limits<int32_t>::max()) + " bytes.");
        return;
    }

    /*
     * If configureEventFlag is true, allocate an additional spot in mGrantor
     * for containing the fd and offset for mmapping the EventFlag word.
     */
    mGrantors.resize(configureEventFlag ? hardware::details::kMinGrantorCountForEvFlagSupport
                                        : hardware::details::kMinGrantorCount);

    size_t memSize[] = {
            sizeof(hardware::details::RingBufferPosition), /* memory to be allocated for read
                                                            * pointer counter
                                                            */
            sizeof(hardware::details::RingBufferPosition), /* memory to be allocated for write
                                                     pointer counter */
            bufferSize,                   /* memory to be allocated for data buffer */
            sizeof(std::atomic<uint32_t>) /* memory to be allocated for EventFlag word */
    };

    /*
     * Create a default grantor descriptor for read, write pointers and
     * the data buffer. fdIndex parameter is set to 0 by default and
     * the offset for each grantor is contiguous.
     */
    for (size_t grantorPos = 0, offset = 0; grantorPos < mGrantors.size();
         offset += memSize[grantorPos++]) {
        mGrantors[grantorPos] = {
                0 /* grantor flags */, 0 /* fdIndex */,
                static_cast<uint32_t>(hardware::details::alignToWordBoundary(offset)),
                memSize[grantorPos]};
    }
}

template <typename T, MQFlavor flavor>
AidlMQDescriptorShim<T, flavor>::~AidlMQDescriptorShim() {
    if (mHandle != nullptr) {
        native_handle_close(mHandle);
        native_handle_delete(mHandle);
    }
}

template <typename T, MQFlavor flavor>
size_t AidlMQDescriptorShim<T, flavor>::getSize() const {
    return mGrantors[hardware::details::DATAPTRPOS].extent;
}

template <typename T, MQFlavor flavor>
size_t AidlMQDescriptorShim<T, flavor>::getQuantum() const {
    return mQuantum;
}

template <typename T, MQFlavor flavor>
uint32_t AidlMQDescriptorShim<T, flavor>::getFlags() const {
    return mFlags;
}

template <typename T, MQFlavor flavor>
std::string toString(const AidlMQDescriptorShim<T, flavor>& q) {
    std::string os;
    if (flavor & hardware::kSynchronizedReadWrite) {
        os += "fmq_sync";
    }
    if (flavor & hardware::kUnsynchronizedWrite) {
        os += "fmq_unsync";
    }
    os += " {" + toString(q.grantors().size()) + " grantor(s), " +
          "size = " + toString(q.getSize()) + ", .handle = " + toString(q.handle()) +
          ", .quantum = " + toString(q.getQuantum()) + "}";
    return os;
}

}  // namespace details
}  // namespace android
