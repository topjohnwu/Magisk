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
#pragma once

#include <aidl/android/hardware/common/fmq/MQDescriptor.h>
#include <aidl/android/hardware/common/fmq/SynchronizedReadWrite.h>
#include <aidl/android/hardware/common/fmq/UnsynchronizedWrite.h>
#include <cutils/native_handle.h>
#include <fmq/AidlMessageQueue.h>
#include <hidl/MQDescriptor.h>

namespace android {
using aidl::android::hardware::common::fmq::GrantorDescriptor;
using aidl::android::hardware::common::fmq::MQDescriptor;
using hardware::details::logError;

/**
 * This function converts a HIDL hardware::MQDescriptor to an AIDL
 * aidl::android::hardware::common::fmq::MQDescriptor for Fast
 * Message Queue.
 *
 * This is considered UNSAFE because it is not checking the offsets of each of the
 * paylod types' fields. In order for these objects to be passed through shared memory safely,
 * they must have the exact same memory layout. Same size, same alignment, and same
 * offsets for each field. Make sure this is the case before using this!
 * Same sized C++ fundamental types and enums with same sized backing types are OK.
 * Ex 1: uint64_t is compatible with int64_t
 * Ex 2:
 * @FixedSize parcelable Foo {
 *   int a;
 *   long b;
 *   MyEnum c; // backed by int32_t
 * }
 * struct Bar {
 *   int a;
 *   long b;
 *   YourEnum c; // backed by uint32_t
 * }
 * The two types above are compatible with each other as long as the fields have
 * the same offsets.
 *
 * Template params:
 *    HidlPayload - the type of the payload used for the HIDL MessageQueue
 *    AidlPayload - the type of the payload used for the AIDL AidlMessageQueue
 *    AidlFlavor - the flavor of the queues. Either SynchronizedReadWrite,
 *                 or UnsynchronizedWrite
 * Function params:
 *    hidlDesc - reference to the HIDL MQDescriptor to be copied from
 *    aidlDesc - pointer to the AIDL MQDescriptor to be copied to
 */
template <typename HidlPayload, typename AidlPayload, typename AidlFlavor>
bool unsafeHidlToAidlMQDescriptor(
        const hardware::MQDescriptor<HidlPayload, FlavorTypeToValue<AidlFlavor>::value>& hidlDesc,
        MQDescriptor<AidlPayload, AidlFlavor>* aidlDesc) {
    static_assert(sizeof(HidlPayload) == sizeof(AidlPayload),
                  "Payload types are definitely incompatible");
    static_assert(alignof(HidlPayload) == alignof(AidlPayload),
                  "Payload types are definitely incompatible");
    STATIC_AIDL_TYPE_CHECK(AidlPayload);
    if (!aidlDesc->grantors.empty()) {
        logError("Destination AIDL MQDescriptor should be empty, but already contains grantors.");
        return false;
    }

    for (const auto& grantor : hidlDesc.grantors()) {
        if (static_cast<int32_t>(grantor.offset) < 0 || static_cast<int64_t>(grantor.extent) < 0 ||
            static_cast<int64_t>(grantor.fdIndex) < 0) {
            logError(
                    "Unsafe static_cast of grantor fields. Either the hardware::MQDescriptor is "
                    "invalid, or the MessageQueue is too large to be described by AIDL.");
            return false;
        }
        aidlDesc->grantors.push_back(
                GrantorDescriptor{.fdIndex = static_cast<int32_t>(grantor.fdIndex),
                                  .offset = static_cast<int32_t>(grantor.offset),
                                  .extent = static_cast<int64_t>(grantor.extent)});
    }

    std::vector<ndk::ScopedFileDescriptor> fds;
    std::vector<int> ints;
    int data_index = 0;
    for (; data_index < hidlDesc.handle()->numFds; data_index++) {
        fds.push_back(ndk::ScopedFileDescriptor(dup(hidlDesc.handle()->data[data_index])));
    }
    for (; data_index < hidlDesc.handle()->numFds + hidlDesc.handle()->numInts; data_index++) {
        ints.push_back(hidlDesc.handle()->data[data_index]);
    }

    aidlDesc->handle = {std::move(fds), std::move(ints)};
    if (static_cast<int32_t>(hidlDesc.getQuantum()) < 0 ||
        static_cast<int32_t>(hidlDesc.getFlags()) < 0) {
        logError(
                "Unsafe static_cast of quantum or flags. Either the hardware::MQDescriptor is "
                "invalid, or the MessageQueue is too large to be described by AIDL.");
        return false;
    }
    if (hidlDesc.getFlags() != FlavorTypeToValue<AidlFlavor>::value) {
        logError("hardware::MQDescriptor hidlDesc is invalid. Unexpected getFlags() value: " +
                 std::to_string(hidlDesc.getFlags()) +
                 ". Expected value: " + std::to_string(FlavorTypeToValue<AidlFlavor>::value));
        return false;
    }
    aidlDesc->quantum = static_cast<int32_t>(hidlDesc.getQuantum());
    aidlDesc->flags = static_cast<int32_t>(hidlDesc.getFlags());
    return true;
}

}  // namespace android
