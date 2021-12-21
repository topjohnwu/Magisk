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

#include "TestAidlMsgQ.h"

namespace aidl {
namespace android {
namespace fmq {
namespace test {

// Methods from ::aidl::android::fmq::test::ITestAidlMsgQ follow.
ndk::ScopedAStatus TestAidlMsgQ::configureFmqSyncReadWrite(
        const MQDescriptor<int32_t, SynchronizedReadWrite>& mqDesc, bool* _aidl_return) {
    mFmqSynchronized.reset(new (std::nothrow) TestAidlMsgQ::MessageQueueSync(mqDesc));
    if ((mFmqSynchronized == nullptr) || (mFmqSynchronized->isValid() == false)) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    /*
     * Initialize the EventFlag word with bit FMQ_NOT_FULL.
     */
    auto evFlagWordPtr = mFmqSynchronized->getEventFlagWord();
    if (evFlagWordPtr != nullptr) {
        std::atomic_init(evFlagWordPtr, static_cast<uint32_t>(EventFlagBits::FMQ_NOT_FULL));
    }
    *_aidl_return = true;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TestAidlMsgQ::getFmqUnsyncWrite(
        bool configureFmq, bool userFd, MQDescriptor<int32_t, UnsynchronizedWrite>* mqDesc,
        bool* _aidl_return) {
    if (configureFmq) {
        static constexpr size_t kNumElementsInQueue = 1024;
        static constexpr size_t kElementSizeBytes = sizeof(int32_t);
        ::android::base::unique_fd ringbufferFd;
        if (userFd) {
            ringbufferFd.reset(
                    ::ashmem_create_region("UnsyncWrite", kNumElementsInQueue * kElementSizeBytes));
        }
        mFmqUnsynchronized.reset(new (std::nothrow) TestAidlMsgQ::MessageQueueUnsync(
                kNumElementsInQueue, false, std::move(ringbufferFd),
                kNumElementsInQueue * kElementSizeBytes));
    }

    if ((mFmqUnsynchronized == nullptr) || (mFmqUnsynchronized->isValid() == false) ||
        (mqDesc == nullptr)) {
        *_aidl_return = false;
    } else {
        *mqDesc = std::move(mFmqUnsynchronized->dupeDesc());
        *_aidl_return = true;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TestAidlMsgQ::requestBlockingRead(int32_t count) {
    std::vector<int32_t> data(count);
    bool result = mFmqSynchronized->readBlocking(
            &data[0], count, static_cast<uint32_t>(EventFlagBits::FMQ_NOT_FULL),
            static_cast<uint32_t>(EventFlagBits::FMQ_NOT_EMPTY), 5000000000 /* timeOutNanos */);

    if (result == false) {
        ALOGE("Blocking read fails");
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TestAidlMsgQ::requestBlockingReadDefaultEventFlagBits(int32_t count) {
    std::vector<int32_t> data(count);
    bool result = mFmqSynchronized->readBlocking(&data[0], count);

    if (result == false) {
        ALOGE("Blocking read fails");
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TestAidlMsgQ::requestBlockingReadRepeat(int32_t count, int32_t numIter) {
    std::vector<int32_t> data(count);
    for (int i = 0; i < numIter; i++) {
        bool result = mFmqSynchronized->readBlocking(
                &data[0], count, static_cast<uint32_t>(EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(EventFlagBits::FMQ_NOT_EMPTY), 5000000000 /* timeOutNanos */);

        if (result == false) {
            ALOGE("Blocking read fails");
            break;
        }
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TestAidlMsgQ::requestReadFmqSync(int32_t count, bool* _aidl_return) {
    std::vector<int32_t> data(count);
    bool result = mFmqSynchronized->read(&data[0], count) && verifyData(&data[0], count);
    *_aidl_return = result;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TestAidlMsgQ::requestReadFmqUnsync(int32_t count, bool* _aidl_return) {
    std::vector<int32_t> data(count);
    bool result = mFmqUnsynchronized->read(&data[0], count) && verifyData(&data[0], count);
    *_aidl_return = result;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TestAidlMsgQ::requestWriteFmqSync(int32_t count, bool* _aidl_return) {
    std::vector<int32_t> data(count);
    for (int i = 0; i < count; i++) {
        data[i] = i;
    }
    bool result = mFmqSynchronized->write(&data[0], count);
    *_aidl_return = result;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TestAidlMsgQ::requestWriteFmqUnsync(int32_t count, bool* _aidl_return) {
    std::vector<int32_t> data(count);
    for (int i = 0; i < count; i++) {
        data[i] = i;
    }
    if (!mFmqUnsynchronized) {
        ALOGE("Unsynchronized queue is not configured.");
        *_aidl_return = false;
        return ndk::ScopedAStatus::ok();
    }
    bool result = mFmqUnsynchronized->write(&data[0], count);
    *_aidl_return = result;
    return ndk::ScopedAStatus::ok();
}

}  // namespace test
}  // namespace fmq
}  // namespace android
}  // namespace aidl
