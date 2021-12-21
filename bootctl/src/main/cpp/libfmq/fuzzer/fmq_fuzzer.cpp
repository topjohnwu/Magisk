/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <stddef.h>
#include <stdint.h>
#include <iostream>
#include <limits>
#include <thread>

#include <android-base/logging.h>
#include <fmq/AidlMessageQueue.h>
#include <fmq/ConvertMQDescriptors.h>
#include <fmq/EventFlag.h>
#include <fmq/MessageQueue.h>

#include "fuzzer/FuzzedDataProvider.h"

using aidl::android::hardware::common::fmq::SynchronizedReadWrite;
using aidl::android::hardware::common::fmq::UnsynchronizedWrite;
using android::hardware::kSynchronizedReadWrite;
using android::hardware::kUnsynchronizedWrite;

typedef int32_t payload_t;

/*
 * MessageQueueBase.h contains asserts when memory allocation fails. So we need
 * to set a reasonable limit if we want to avoid those asserts.
 */
static constexpr size_t kAlignment = 8;
static constexpr size_t kMaxNumElements = PAGE_SIZE * 10 / sizeof(payload_t) - kAlignment + 1;

/*
 * The read counter can be found in the shared memory 16 bytes before the start
 * of the ring buffer.
 */
static constexpr int kReadCounterOffsetBytes = 16;
/*
 * The write counter can be found in the shared memory 8 bytes before the start
 * of the ring buffer.
 */
static constexpr int kWriteCounterOffsetBytes = 8;

static constexpr int kMaxNumSyncReaders = 1;
static constexpr int kMaxNumUnsyncReaders = 5;

typedef android::AidlMessageQueue<payload_t, SynchronizedReadWrite> AidlMessageQueueSync;
typedef android::AidlMessageQueue<payload_t, UnsynchronizedWrite> AidlMessageQueueUnsync;
typedef android::hardware::MessageQueue<payload_t, kSynchronizedReadWrite> MessageQueueSync;
typedef android::hardware::MessageQueue<payload_t, kUnsynchronizedWrite> MessageQueueUnsync;
typedef aidl::android::hardware::common::fmq::MQDescriptor<payload_t, SynchronizedReadWrite>
        AidlMQDescSync;
typedef aidl::android::hardware::common::fmq::MQDescriptor<payload_t, UnsynchronizedWrite>
        AidlMQDescUnsync;
typedef android::hardware::MQDescriptorSync<payload_t> MQDescSync;
typedef android::hardware::MQDescriptorUnsync<payload_t> MQDescUnsync;

template <typename Queue, typename Desc>
void reader(const Desc& desc, std::vector<uint8_t> readerData) {
    Queue readMq(desc);
    if (!readMq.isValid()) {
        LOG(ERROR) << "read mq invalid";
        return;
    }
    FuzzedDataProvider fdp(&readerData[0], readerData.size());
    while (fdp.remaining_bytes()) {
        typename Queue::MemTransaction tx;
        size_t numElements = fdp.ConsumeIntegralInRange<size_t>(0, kMaxNumElements);
        if (!readMq.beginRead(numElements, &tx)) {
            continue;
        }
        const auto& region = tx.getFirstRegion();
        payload_t* firstStart = region.getAddress();

        // TODO add the debug function to get pointer to the ring buffer
        uint64_t* writeCounter = reinterpret_cast<uint64_t*>(
                reinterpret_cast<uint8_t*>(firstStart) - kWriteCounterOffsetBytes);
        *writeCounter = fdp.ConsumeIntegral<uint64_t>();

        (void)std::to_string(*firstStart);

        readMq.commitRead(numElements);
    }
}

template <typename Queue>
void writer(Queue& writeMq, FuzzedDataProvider& fdp) {
    while (fdp.remaining_bytes()) {
        typename Queue::MemTransaction tx;
        size_t numElements = 1;
        if (!writeMq.beginWrite(numElements, &tx)) {
            // need to consume something so we don't end up looping forever
            fdp.ConsumeIntegral<uint8_t>();
            continue;
        }

        const auto& region = tx.getFirstRegion();
        payload_t* firstStart = region.getAddress();

        // TODO add the debug function to get pointer to the ring buffer
        uint64_t* readCounter = reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(firstStart) -
                                                            kReadCounterOffsetBytes);
        *readCounter = fdp.ConsumeIntegral<uint64_t>();

        *firstStart = fdp.ConsumeIntegral<payload_t>();

        writeMq.commitWrite(numElements);
    }
}

template <typename Queue, typename Desc>
void fuzzAidlWithReaders(std::vector<uint8_t>& writerData,
                         std::vector<std::vector<uint8_t>>& readerData) {
    FuzzedDataProvider fdp(&writerData[0], writerData.size());
    Queue writeMq(fdp.ConsumeIntegralInRange<size_t>(1, kMaxNumElements), fdp.ConsumeBool());
    if (!writeMq.isValid()) {
        LOG(ERROR) << "AIDL write mq invalid";
        return;
    }
    const auto desc = writeMq.dupeDesc();
    CHECK(desc.handle.fds[0].get() != -1);

    std::vector<std::thread> clients;
    for (int i = 0; i < readerData.size(); i++) {
        clients.emplace_back(reader<Queue, Desc>, std::ref(desc), std::ref(readerData[i]));
    }

    writer<Queue>(writeMq, fdp);

    for (auto& client : clients) {
        client.join();
    }
}

template <typename Queue, typename Desc>
void fuzzHidlWithReaders(std::vector<uint8_t>& writerData,
                         std::vector<std::vector<uint8_t>>& readerData) {
    FuzzedDataProvider fdp(&writerData[0], writerData.size());
    Queue writeMq(fdp.ConsumeIntegralInRange<size_t>(1, kMaxNumElements), fdp.ConsumeBool());
    if (!writeMq.isValid()) {
        LOG(ERROR) << "HIDL write mq invalid";
        return;
    }
    const auto desc = writeMq.getDesc();
    CHECK(desc->isHandleValid());

    std::vector<std::thread> clients;
    for (int i = 0; i < readerData.size(); i++) {
        clients.emplace_back(reader<Queue, Desc>, std::ref(*desc), std::ref(readerData[i]));
    }

    writer<Queue>(writeMq, fdp);

    for (auto& client : clients) {
        client.join();
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 1 || size > 50000) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);

    bool fuzzSync = fdp.ConsumeBool();
    std::vector<std::vector<uint8_t>> readerData;
    uint8_t numReaders = fuzzSync ? fdp.ConsumeIntegralInRange<uint8_t>(0, kMaxNumSyncReaders)
                                  : fdp.ConsumeIntegralInRange<uint8_t>(0, kMaxNumUnsyncReaders);
    for (int i = 0; i < numReaders; i++) {
        readerData.emplace_back(fdp.ConsumeBytes<uint8_t>(5));
    }
    std::vector<uint8_t> writerData = fdp.ConsumeRemainingBytes<uint8_t>();

    if (fuzzSync) {
        fuzzHidlWithReaders<MessageQueueSync, MQDescSync>(writerData, readerData);
        fuzzAidlWithReaders<AidlMessageQueueSync, AidlMQDescSync>(writerData, readerData);
    } else {
        fuzzHidlWithReaders<MessageQueueUnsync, MQDescUnsync>(writerData, readerData);
        fuzzAidlWithReaders<AidlMessageQueueUnsync, AidlMQDescUnsync>(writerData, readerData);
    }

    return 0;
}
