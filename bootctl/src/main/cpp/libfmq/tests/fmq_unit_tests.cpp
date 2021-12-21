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

#include <asm-generic/mman.h>
#include <fmq/AidlMessageQueue.h>
#include <fmq/ConvertMQDescriptors.h>
#include <fmq/EventFlag.h>
#include <fmq/MessageQueue.h>
#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>
#include <sys/resource.h>
#include <atomic>
#include <cstdlib>
#include <sstream>
#include <thread>

using aidl::android::hardware::common::fmq::SynchronizedReadWrite;
using aidl::android::hardware::common::fmq::UnsynchronizedWrite;
using android::hardware::kSynchronizedReadWrite;
using android::hardware::kUnsynchronizedWrite;

enum EventFlagBits : uint32_t {
    kFmqNotEmpty = 1 << 0,
    kFmqNotFull = 1 << 1,
};

typedef android::AidlMessageQueue<uint8_t, SynchronizedReadWrite> AidlMessageQueueSync;
typedef android::AidlMessageQueue<uint8_t, UnsynchronizedWrite> AidlMessageQueueUnsync;
typedef android::hardware::MessageQueue<uint8_t, kSynchronizedReadWrite> MessageQueueSync;
typedef android::hardware::MessageQueue<uint8_t, kUnsynchronizedWrite> MessageQueueUnsync;
typedef android::AidlMessageQueue<uint16_t, SynchronizedReadWrite> AidlMessageQueueSync16;
typedef android::hardware::MessageQueue<uint16_t, kSynchronizedReadWrite> MessageQueueSync16;

typedef android::hardware::MessageQueue<uint8_t, kSynchronizedReadWrite> MessageQueueSync8;
typedef android::hardware::MQDescriptor<uint8_t, kSynchronizedReadWrite> HidlMQDescSync8;
typedef android::AidlMessageQueue<int8_t, SynchronizedReadWrite> AidlMessageQueueSync8;
typedef aidl::android::hardware::common::fmq::MQDescriptor<int8_t, SynchronizedReadWrite>
        AidlMQDescSync8;
typedef android::hardware::MessageQueue<uint8_t, kUnsynchronizedWrite> MessageQueueUnsync8;
typedef android::hardware::MQDescriptor<uint8_t, kUnsynchronizedWrite> HidlMQDescUnsync8;
typedef android::AidlMessageQueue<int8_t, UnsynchronizedWrite> AidlMessageQueueUnsync8;
typedef aidl::android::hardware::common::fmq::MQDescriptor<int8_t, UnsynchronizedWrite>
        AidlMQDescUnsync8;

enum class SetupType {
    SINGLE_FD,
    DOUBLE_FD,
};

template <typename T, SetupType setupType>
class TestParamTypes {
  public:
    typedef T MQType;
    static constexpr SetupType Setup = setupType;
};

// Run everything on both the AIDL and HIDL versions with one and two FDs
typedef ::testing::Types<TestParamTypes<AidlMessageQueueSync, SetupType::SINGLE_FD>,
                         TestParamTypes<MessageQueueSync, SetupType::SINGLE_FD>,
                         TestParamTypes<AidlMessageQueueSync, SetupType::DOUBLE_FD>,
                         TestParamTypes<MessageQueueSync, SetupType::DOUBLE_FD>>
        SyncTypes;
typedef ::testing::Types<TestParamTypes<AidlMessageQueueUnsync, SetupType::SINGLE_FD>,
                         TestParamTypes<MessageQueueUnsync, SetupType::SINGLE_FD>,
                         TestParamTypes<AidlMessageQueueUnsync, SetupType::DOUBLE_FD>,
                         TestParamTypes<MessageQueueUnsync, SetupType::DOUBLE_FD>>
        UnsyncTypes;
typedef ::testing::Types<TestParamTypes<AidlMessageQueueSync16, SetupType::SINGLE_FD>,
                         TestParamTypes<MessageQueueSync16, SetupType::SINGLE_FD>,
                         TestParamTypes<AidlMessageQueueSync16, SetupType::DOUBLE_FD>,
                         TestParamTypes<MessageQueueSync16, SetupType::DOUBLE_FD>>
        BadConfigTypes;

template <typename T>
class TestBase : public ::testing::Test {
  public:
    static void ReaderThreadBlocking(typename T::MQType* fmq, std::atomic<uint32_t>* fwAddr);
    static void ReaderThreadBlocking2(typename T::MQType* fmq, std::atomic<uint32_t>* fwAddr);
};

TYPED_TEST_CASE(SynchronizedReadWrites, SyncTypes);

template <typename T>
class SynchronizedReadWrites : public TestBase<T> {
  protected:
    virtual void TearDown() {
        delete mQueue;
    }

    virtual void SetUp() {
        static constexpr size_t kNumElementsInQueue = 2048;
        static constexpr size_t kPayloadSizeBytes = 1;
        if (T::Setup == SetupType::SINGLE_FD) {
            mQueue = new (std::nothrow) typename T::MQType(kNumElementsInQueue);
        } else {
            android::base::unique_fd ringbufferFd(::ashmem_create_region(
                    "SyncReadWrite", kNumElementsInQueue * kPayloadSizeBytes));
            mQueue = new (std::nothrow)
                    typename T::MQType(kNumElementsInQueue, false, std::move(ringbufferFd),
                                       kNumElementsInQueue * kPayloadSizeBytes);
        }
        ASSERT_NE(nullptr, mQueue);
        ASSERT_TRUE(mQueue->isValid());
        mNumMessagesMax = mQueue->getQuantumCount();
        ASSERT_EQ(kNumElementsInQueue, mNumMessagesMax);
    }

    typename T::MQType* mQueue = nullptr;
    size_t mNumMessagesMax = 0;
};

TYPED_TEST_CASE(UnsynchronizedWriteTest, UnsyncTypes);

template <typename T>
class UnsynchronizedWriteTest : public TestBase<T> {
  protected:
    virtual void TearDown() {
        delete mQueue;
    }

    virtual void SetUp() {
        static constexpr size_t kNumElementsInQueue = 2048;
        static constexpr size_t kPayloadSizeBytes = 1;
        if (T::Setup == SetupType::SINGLE_FD) {
            mQueue = new (std::nothrow) typename T::MQType(kNumElementsInQueue);
        } else {
            android::base::unique_fd ringbufferFd(
                    ::ashmem_create_region("UnsyncWrite", kNumElementsInQueue * kPayloadSizeBytes));
            mQueue = new (std::nothrow)
                    typename T::MQType(kNumElementsInQueue, false, std::move(ringbufferFd),
                                       kNumElementsInQueue * kPayloadSizeBytes);
        }
        ASSERT_NE(nullptr, mQueue);
        ASSERT_TRUE(mQueue->isValid());
        mNumMessagesMax = mQueue->getQuantumCount();
        ASSERT_EQ(kNumElementsInQueue, mNumMessagesMax);
    }

    typename T::MQType* mQueue = nullptr;
    size_t mNumMessagesMax = 0;
};

TYPED_TEST_CASE(BlockingReadWrites, SyncTypes);

template <typename T>
class BlockingReadWrites : public TestBase<T> {
  protected:
    virtual void TearDown() {
        delete mQueue;
    }
    virtual void SetUp() {
        static constexpr size_t kNumElementsInQueue = 2048;
        static constexpr size_t kPayloadSizeBytes = 1;
        if (T::Setup == SetupType::SINGLE_FD) {
            mQueue = new (std::nothrow) typename T::MQType(kNumElementsInQueue);
        } else {
            android::base::unique_fd ringbufferFd(::ashmem_create_region(
                    "SyncBlockingReadWrite", kNumElementsInQueue * kPayloadSizeBytes));
            mQueue = new (std::nothrow)
                    typename T::MQType(kNumElementsInQueue, false, std::move(ringbufferFd),
                                       kNumElementsInQueue * kPayloadSizeBytes);
        }
        ASSERT_NE(nullptr, mQueue);
        ASSERT_TRUE(mQueue->isValid());
        mNumMessagesMax = mQueue->getQuantumCount();
        ASSERT_EQ(kNumElementsInQueue, mNumMessagesMax);
        /*
         * Initialize the EventFlag word to indicate Queue is not full.
         */
        std::atomic_init(&mFw, static_cast<uint32_t>(kFmqNotFull));
    }

    typename T::MQType* mQueue;
    std::atomic<uint32_t> mFw;
    size_t mNumMessagesMax = 0;
};

TYPED_TEST_CASE(QueueSizeOdd, SyncTypes);

template <typename T>
class QueueSizeOdd : public TestBase<T> {
  protected:
    virtual void TearDown() { delete mQueue; }
    virtual void SetUp() {
        static constexpr size_t kNumElementsInQueue = 2049;
        static constexpr size_t kPayloadSizeBytes = 1;
        if (T::Setup == SetupType::SINGLE_FD) {
            mQueue = new (std::nothrow)
                    typename T::MQType(kNumElementsInQueue, true /* configureEventFlagWord */);
        } else {
            android::base::unique_fd ringbufferFd(
                    ::ashmem_create_region("SyncSizeOdd", kNumElementsInQueue * kPayloadSizeBytes));
            mQueue = new (std::nothrow) typename T::MQType(
                    kNumElementsInQueue, true /* configureEventFlagWord */, std::move(ringbufferFd),
                    kNumElementsInQueue * kPayloadSizeBytes);
        }
        ASSERT_NE(nullptr, mQueue);
        ASSERT_TRUE(mQueue->isValid());
        mNumMessagesMax = mQueue->getQuantumCount();
        ASSERT_EQ(kNumElementsInQueue, mNumMessagesMax);
        auto evFlagWordPtr = mQueue->getEventFlagWord();
        ASSERT_NE(nullptr, evFlagWordPtr);
        /*
         * Initialize the EventFlag word to indicate Queue is not full.
         */
        std::atomic_init(evFlagWordPtr, static_cast<uint32_t>(kFmqNotFull));
    }

    typename T::MQType* mQueue;
    size_t mNumMessagesMax = 0;
};

TYPED_TEST_CASE(BadQueueConfig, BadConfigTypes);

template <typename T>
class BadQueueConfig : public TestBase<T> {};

class AidlOnlyBadQueueConfig : public ::testing::Test {};
class Hidl2AidlOperation : public ::testing::Test {};
class DoubleFdFailures : public ::testing::Test {};

/*
 * Utility function to initialize data to be written to the FMQ
 */
inline void initData(uint8_t* data, size_t count) {
    for (size_t i = 0; i < count; i++) {
        data[i] = i & 0xFF;
    }
}

/*
 * This thread will attempt to read and block. When wait returns
 * it checks if the kFmqNotEmpty bit is actually set.
 * If the read is succesful, it signals Wake to kFmqNotFull.
 */
template <typename T>
void TestBase<T>::ReaderThreadBlocking(typename T::MQType* fmq, std::atomic<uint32_t>* fwAddr) {
    const size_t dataLen = 64;
    uint8_t data[dataLen];
    android::hardware::EventFlag* efGroup = nullptr;
    android::status_t status = android::hardware::EventFlag::createEventFlag(fwAddr, &efGroup);
    ASSERT_EQ(android::NO_ERROR, status);
    ASSERT_NE(nullptr, efGroup);

    while (true) {
        uint32_t efState = 0;
        android::status_t ret = efGroup->wait(kFmqNotEmpty,
                                              &efState,
                                              5000000000 /* timeoutNanoSeconds */);
        /*
         * Wait should not time out here after 5s
         */
        ASSERT_NE(android::TIMED_OUT, ret);

        if ((efState & kFmqNotEmpty) && fmq->read(data, dataLen)) {
            efGroup->wake(kFmqNotFull);
            break;
        }
    }

    status = android::hardware::EventFlag::deleteEventFlag(&efGroup);
    ASSERT_EQ(android::NO_ERROR, status);
}

/*
 * This thread will attempt to read and block using the readBlocking() API and
 * passes in a pointer to an EventFlag object.
 */
template <typename T>
void TestBase<T>::ReaderThreadBlocking2(typename T::MQType* fmq, std::atomic<uint32_t>* fwAddr) {
    const size_t dataLen = 64;
    uint8_t data[dataLen];
    android::hardware::EventFlag* efGroup = nullptr;
    android::status_t status = android::hardware::EventFlag::createEventFlag(fwAddr, &efGroup);
    ASSERT_EQ(android::NO_ERROR, status);
    ASSERT_NE(nullptr, efGroup);
    bool ret = fmq->readBlocking(data,
                                 dataLen,
                                 static_cast<uint32_t>(kFmqNotFull),
                                 static_cast<uint32_t>(kFmqNotEmpty),
                                 5000000000 /* timeOutNanos */,
                                 efGroup);
    ASSERT_TRUE(ret);
    status = android::hardware::EventFlag::deleteEventFlag(&efGroup);
    ASSERT_EQ(android::NO_ERROR, status);
}

TYPED_TEST(BadQueueConfig, QueueSizeTooLarge) {
    size_t numElementsInQueue = SIZE_MAX / sizeof(uint16_t) + 1;
    typename TypeParam::MQType* fmq =
            new (std::nothrow) typename TypeParam::MQType(numElementsInQueue);
    ASSERT_NE(nullptr, fmq);
    /*
     * Should fail due to size being too large to fit into size_t.
     */
    ASSERT_FALSE(fmq->isValid());
}

// If this test fails and we do leak FDs, the next test will cause a crash
TEST_F(AidlOnlyBadQueueConfig, LookForLeakedFds) {
    size_t numElementsInQueue = SIZE_MAX / sizeof(uint32_t) - PAGE_SIZE - 1;
    struct rlimit rlim;
    ASSERT_EQ(getrlimit(RLIMIT_NOFILE, &rlim), 0);
    for (int i = 0; i <= rlim.rlim_cur + 1; i++) {
        android::AidlMessageQueue<uint32_t, SynchronizedReadWrite> fmq(numElementsInQueue);
        ASSERT_FALSE(fmq.isValid());
    }
    // try to get another FD
    int fd = ashmem_create_region("test", 100);
    ASSERT_NE(fd, -1);
    close(fd);
}

TEST_F(Hidl2AidlOperation, ConvertDescriptorsSync) {
    size_t numElementsInQueue = 64;

    // Create HIDL side and get MQDescriptor
    MessageQueueSync8* fmq = new (std::nothrow) MessageQueueSync8(numElementsInQueue);
    ASSERT_NE(nullptr, fmq);
    ASSERT_TRUE(fmq->isValid());
    const HidlMQDescSync8* hidlDesc = fmq->getDesc();
    ASSERT_NE(nullptr, hidlDesc);

    // Create AIDL MQDescriptor to send to another process based off the HIDL MQDescriptor
    AidlMQDescSync8 aidlDesc;
    android::unsafeHidlToAidlMQDescriptor<uint8_t, int8_t, SynchronizedReadWrite>(*hidlDesc,
                                                                                  &aidlDesc);

    // Other process will create the other side of the queue using the AIDL MQDescriptor
    AidlMessageQueueSync8* aidlFmq = new (std::nothrow) AidlMessageQueueSync8(aidlDesc);
    ASSERT_NE(nullptr, aidlFmq);
    ASSERT_TRUE(aidlFmq->isValid());

    // Make sure a write to the HIDL side, will show up for the AIDL side
    constexpr size_t dataLen = 4;
    uint8_t data[dataLen] = {12, 11, 10, 9};
    fmq->write(data, dataLen);

    int8_t readData[dataLen];
    ASSERT_TRUE(aidlFmq->read(readData, dataLen));

    ASSERT_EQ(data[0], readData[0]);
    ASSERT_EQ(data[1], readData[1]);
    ASSERT_EQ(data[2], readData[2]);
    ASSERT_EQ(data[3], readData[3]);
}

TEST_F(Hidl2AidlOperation, ConvertDescriptorsUnsync) {
    size_t numElementsInQueue = 64;

    // Create HIDL side and get MQDescriptor
    MessageQueueUnsync8* fmq = new (std::nothrow) MessageQueueUnsync8(numElementsInQueue);
    ASSERT_NE(nullptr, fmq);
    ASSERT_TRUE(fmq->isValid());
    const HidlMQDescUnsync8* hidlDesc = fmq->getDesc();
    ASSERT_NE(nullptr, hidlDesc);

    // Create AIDL MQDescriptor to send to another process based off the HIDL MQDescriptor
    AidlMQDescUnsync8 aidlDesc;
    android::unsafeHidlToAidlMQDescriptor<uint8_t, int8_t, UnsynchronizedWrite>(*hidlDesc,
                                                                                &aidlDesc);

    // Other process will create the other side of the queue using the AIDL MQDescriptor
    AidlMessageQueueUnsync8* aidlFmq = new (std::nothrow) AidlMessageQueueUnsync8(aidlDesc);
    ASSERT_NE(nullptr, aidlFmq);
    ASSERT_TRUE(aidlFmq->isValid());

    // Can have multiple readers with unsync flavor
    AidlMessageQueueUnsync8* aidlFmq2 = new (std::nothrow) AidlMessageQueueUnsync8(aidlDesc);
    ASSERT_NE(nullptr, aidlFmq2);
    ASSERT_TRUE(aidlFmq2->isValid());

    // Make sure a write to the HIDL side, will show up for the AIDL side
    constexpr size_t dataLen = 4;
    uint8_t data[dataLen] = {12, 11, 10, 9};
    fmq->write(data, dataLen);

    int8_t readData[dataLen];
    ASSERT_TRUE(aidlFmq->read(readData, dataLen));
    int8_t readData2[dataLen];
    ASSERT_TRUE(aidlFmq2->read(readData2, dataLen));

    ASSERT_EQ(data[0], readData[0]);
    ASSERT_EQ(data[1], readData[1]);
    ASSERT_EQ(data[2], readData[2]);
    ASSERT_EQ(data[3], readData[3]);
    ASSERT_EQ(data[0], readData2[0]);
    ASSERT_EQ(data[1], readData2[1]);
    ASSERT_EQ(data[2], readData2[2]);
    ASSERT_EQ(data[3], readData2[3]);
}

TEST_F(Hidl2AidlOperation, ConvertFdIndex1) {
    native_handle_t* mqHandle = native_handle_create(2 /* numFds */, 0 /* numInts */);
    if (mqHandle == nullptr) {
        return;
    }
    mqHandle->data[0] = 12;
    mqHandle->data[1] = 5;
    ::android::hardware::hidl_vec<android::hardware::GrantorDescriptor> grantors;
    grantors.resize(3);
    grantors[0] = {0, 1 /* fdIndex */, 16, 16};
    grantors[1] = {0, 1 /* fdIndex */, 16, 16};
    grantors[2] = {0, 1 /* fdIndex */, 16, 16};

    HidlMQDescUnsync8* hidlDesc = new (std::nothrow) HidlMQDescUnsync8(grantors, mqHandle, 10);
    ASSERT_TRUE(hidlDesc->isHandleValid());

    AidlMQDescUnsync8 aidlDesc;
    bool ret = android::unsafeHidlToAidlMQDescriptor<uint8_t, int8_t, UnsynchronizedWrite>(
            *hidlDesc, &aidlDesc);
    ASSERT_TRUE(ret);
}

TEST_F(Hidl2AidlOperation, ConvertMultipleFds) {
    native_handle_t* mqHandle = native_handle_create(2 /* numFds */, 0 /* numInts */);
    if (mqHandle == nullptr) {
        return;
    }
    mqHandle->data[0] = ::ashmem_create_region("ConvertMultipleFds", 8);
    mqHandle->data[1] = ::ashmem_create_region("ConvertMultipleFds2", 8);
    ::android::hardware::hidl_vec<android::hardware::GrantorDescriptor> grantors;
    grantors.resize(3);
    grantors[0] = {0, 1 /* fdIndex */, 16, 16};
    grantors[1] = {0, 1 /* fdIndex */, 16, 16};
    grantors[2] = {0, 0 /* fdIndex */, 16, 16};

    HidlMQDescUnsync8* hidlDesc = new (std::nothrow) HidlMQDescUnsync8(grantors, mqHandle, 10);
    ASSERT_TRUE(hidlDesc->isHandleValid());

    AidlMQDescUnsync8 aidlDesc;
    bool ret = android::unsafeHidlToAidlMQDescriptor<uint8_t, int8_t, UnsynchronizedWrite>(
            *hidlDesc, &aidlDesc);
    ASSERT_TRUE(ret);
    EXPECT_EQ(aidlDesc.handle.fds.size(), 2);
    native_handle_close(mqHandle);
    native_handle_delete(mqHandle);
}

// TODO(b/165674950) Since AIDL does not support unsigned integers, it can only support
// 1/2 the queue size of HIDL. Once support is added to AIDL, this restriction can be
// lifted. Until then, check against SSIZE_MAX instead of SIZE_MAX.
TEST_F(AidlOnlyBadQueueConfig, QueueSizeTooLargeForAidl) {
    size_t numElementsInQueue = SSIZE_MAX / sizeof(uint16_t) + 1;
    AidlMessageQueueSync16* fmq = new (std::nothrow) AidlMessageQueueSync16(numElementsInQueue);
    ASSERT_NE(nullptr, fmq);
    /*
     * Should fail due to size being too large to fit into size_t.
     */
    ASSERT_FALSE(fmq->isValid());
}

TEST_F(AidlOnlyBadQueueConfig, NegativeAidlDescriptor) {
    aidl::android::hardware::common::fmq::MQDescriptor<uint16_t, SynchronizedReadWrite> desc;
    desc.quantum = -10;
    AidlMessageQueueSync16* fmq = new (std::nothrow) AidlMessageQueueSync16(desc);
    ASSERT_NE(nullptr, fmq);
    /*
     * Should fail due to quantum being negative.
     */
    ASSERT_FALSE(fmq->isValid());
}

TEST_F(AidlOnlyBadQueueConfig, NegativeAidlDescriptorGrantor) {
    aidl::android::hardware::common::fmq::MQDescriptor<uint16_t, SynchronizedReadWrite> desc;
    desc.quantum = 2;
    desc.flags = 0;
    desc.grantors.push_back(
            aidl::android::hardware::common::fmq::GrantorDescriptor{.offset = 12, .extent = -10});
    AidlMessageQueueSync16* fmq = new (std::nothrow) AidlMessageQueueSync16(desc);
    ASSERT_NE(nullptr, fmq);
    /*
     * Should fail due to grantor having negative extent.
     */
    ASSERT_FALSE(fmq->isValid());
}

/*
 * Test creating a new queue from a modified MQDescriptor of another queue.
 * If MQDescriptor.quantum doesn't match the size of the payload(T), the queue
 * should be invalid.
 */
TEST_F(AidlOnlyBadQueueConfig, MismatchedPayloadSize) {
    AidlMessageQueueSync16 fmq = AidlMessageQueueSync16(64);
    aidl::android::hardware::common::fmq::MQDescriptor<uint16_t, SynchronizedReadWrite> desc =
            fmq.dupeDesc();
    // This should work fine with the unmodified MQDescriptor
    AidlMessageQueueSync16 fmq2 = AidlMessageQueueSync16(desc);
    ASSERT_TRUE(fmq2.isValid());

    // Simulate a difference in payload size between processes handling the queue
    desc.quantum = 8;
    AidlMessageQueueSync16 fmq3 = AidlMessageQueueSync16(desc);

    // Should fail due to the quantum not matching the sizeof(uint16_t)
    ASSERT_FALSE(fmq3.isValid());
}

/*
 * Test creating a new queue with an invalid fd. This should assert with message
 * "mRing is null".
 */
TEST_F(DoubleFdFailures, InvalidFd) {
    EXPECT_DEATH_IF_SUPPORTED(AidlMessageQueueSync(64, false, android::base::unique_fd(3000), 64),
                              "mRing is null");
}

/*
 * Test creating a new queue with a buffer fd and bufferSize smaller than the
 * requested queue. This should fail to create a valid message queue.
 */
TEST_F(DoubleFdFailures, InvalidFdSize) {
    constexpr size_t kNumElementsInQueue = 1024;
    constexpr size_t kRequiredDataBufferSize = kNumElementsInQueue * sizeof(uint16_t);
    android::base::unique_fd ringbufferFd(
            ::ashmem_create_region("SyncReadWrite", kRequiredDataBufferSize - 8));
    AidlMessageQueueSync16 fmq = AidlMessageQueueSync16(
            kNumElementsInQueue, false, std::move(ringbufferFd), kRequiredDataBufferSize - 8);
    EXPECT_FALSE(fmq.isValid());
}

/*
 * Test creating a new queue with a buffer fd and bufferSize larger than the
 * requested queue. The message queue should be valid.
 */
TEST_F(DoubleFdFailures, LargerFdSize) {
    constexpr size_t kNumElementsInQueue = 1024;
    constexpr size_t kRequiredDataBufferSize = kNumElementsInQueue * sizeof(uint16_t);
    android::base::unique_fd ringbufferFd(
            ::ashmem_create_region("SyncReadWrite", kRequiredDataBufferSize + 8));
    AidlMessageQueueSync16 fmq = AidlMessageQueueSync16(
            kNumElementsInQueue, false, std::move(ringbufferFd), kRequiredDataBufferSize + 8);
    EXPECT_TRUE(fmq.isValid());
}

/*
 * Test that basic blocking works. This test uses the non-blocking read()/write()
 * APIs.
 */
TYPED_TEST(BlockingReadWrites, SmallInputTest1) {
    const size_t dataLen = 64;
    uint8_t data[dataLen] = {0};

    android::hardware::EventFlag* efGroup = nullptr;
    android::status_t status = android::hardware::EventFlag::createEventFlag(&this->mFw, &efGroup);

    ASSERT_EQ(android::NO_ERROR, status);
    ASSERT_NE(nullptr, efGroup);

    /*
     * Start a thread that will try to read and block on kFmqNotEmpty.
     */
    std::thread Reader(BlockingReadWrites<TypeParam>::ReaderThreadBlocking, this->mQueue,
                       &this->mFw);
    struct timespec waitTime = {0, 100 * 1000000};
    ASSERT_EQ(0, nanosleep(&waitTime, NULL));

    /*
     * After waiting for some time write into the FMQ
     * and call Wake on kFmqNotEmpty.
     */
    ASSERT_TRUE(this->mQueue->write(data, dataLen));
    status = efGroup->wake(kFmqNotEmpty);
    ASSERT_EQ(android::NO_ERROR, status);

    ASSERT_EQ(0, nanosleep(&waitTime, NULL));
    Reader.join();

    status = android::hardware::EventFlag::deleteEventFlag(&efGroup);
    ASSERT_EQ(android::NO_ERROR, status);
}

/*
 * Test that basic blocking works. This test uses the
 * writeBlocking()/readBlocking() APIs.
 */
TYPED_TEST(BlockingReadWrites, SmallInputTest2) {
    const size_t dataLen = 64;
    uint8_t data[dataLen] = {0};

    android::hardware::EventFlag* efGroup = nullptr;
    android::status_t status = android::hardware::EventFlag::createEventFlag(&this->mFw, &efGroup);

    ASSERT_EQ(android::NO_ERROR, status);
    ASSERT_NE(nullptr, efGroup);

    /*
     * Start a thread that will try to read and block on kFmqNotEmpty. It will
     * call wake() on kFmqNotFull when the read is successful.
     */
    std::thread Reader(BlockingReadWrites<TypeParam>::ReaderThreadBlocking2, this->mQueue,
                       &this->mFw);
    bool ret = this->mQueue->writeBlocking(data, dataLen, static_cast<uint32_t>(kFmqNotFull),
                                           static_cast<uint32_t>(kFmqNotEmpty),
                                           5000000000 /* timeOutNanos */, efGroup);
    ASSERT_TRUE(ret);
    Reader.join();

    status = android::hardware::EventFlag::deleteEventFlag(&efGroup);
    ASSERT_EQ(android::NO_ERROR, status);
}

/*
 * Test that basic blocking times out as intended.
 */
TYPED_TEST(BlockingReadWrites, BlockingTimeOutTest) {
    android::hardware::EventFlag* efGroup = nullptr;
    android::status_t status = android::hardware::EventFlag::createEventFlag(&this->mFw, &efGroup);

    ASSERT_EQ(android::NO_ERROR, status);
    ASSERT_NE(nullptr, efGroup);

    /* Block on an EventFlag bit that no one will wake and time out in 1s */
    uint32_t efState = 0;
    android::status_t ret = efGroup->wait(kFmqNotEmpty,
                                          &efState,
                                          1000000000 /* timeoutNanoSeconds */);
    /*
     * Wait should time out in a second.
     */
    EXPECT_EQ(android::TIMED_OUT, ret);

    status = android::hardware::EventFlag::deleteEventFlag(&efGroup);
    ASSERT_EQ(android::NO_ERROR, status);
}

/*
 * Test that odd queue sizes do not cause unaligned error
 * on access to EventFlag object.
 */
TYPED_TEST(QueueSizeOdd, EventFlagTest) {
    const size_t dataLen = 64;
    uint8_t data[dataLen] = {0};

    bool ret = this->mQueue->writeBlocking(data, dataLen, static_cast<uint32_t>(kFmqNotFull),
                                           static_cast<uint32_t>(kFmqNotEmpty),
                                           5000000000 /* timeOutNanos */);
    ASSERT_TRUE(ret);
}

/*
 * Verify that a few bytes of data can be successfully written and read.
 */
TYPED_TEST(SynchronizedReadWrites, SmallInputTest1) {
    const size_t dataLen = 16;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    uint8_t data[dataLen];

    initData(data, dataLen);

    ASSERT_TRUE(this->mQueue->write(data, dataLen));
    uint8_t readData[dataLen] = {};
    ASSERT_TRUE(this->mQueue->read(readData, dataLen));
    ASSERT_EQ(0, memcmp(data, readData, dataLen));
}

/*
 * Verify that a few bytes of data can be successfully written and read using
 * beginRead/beginWrite/CommitRead/CommitWrite
 */
TYPED_TEST(SynchronizedReadWrites, SmallInputTest2) {
    const size_t dataLen = 16;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    uint8_t data[dataLen];

    initData(data, dataLen);

    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_TRUE(this->mQueue->beginWrite(dataLen, &tx));

    ASSERT_TRUE(tx.copyTo(data, 0 /* startIdx */, dataLen));

    ASSERT_TRUE(this->mQueue->commitWrite(dataLen));

    uint8_t readData[dataLen] = {};

    ASSERT_TRUE(this->mQueue->beginRead(dataLen, &tx));

    ASSERT_TRUE(tx.copyFrom(readData, 0 /* startIdx */, dataLen));

    ASSERT_TRUE(this->mQueue->commitRead(dataLen));

    ASSERT_EQ(0, memcmp(data, readData, dataLen));
}

/*
 * Verify that a few bytes of data can be successfully written and read using
 * beginRead/beginWrite/CommitRead/CommitWrite as well as getSlot().
 */
TYPED_TEST(SynchronizedReadWrites, SmallInputTest3) {
    const size_t dataLen = 16;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    uint8_t data[dataLen];

    initData(data, dataLen);
    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_TRUE(this->mQueue->beginWrite(dataLen, &tx));

    auto first = tx.getFirstRegion();
    auto second = tx.getSecondRegion();

    ASSERT_EQ(first.getLength() + second.getLength(),  dataLen);
    for (size_t i = 0; i < dataLen; i++) {
        uint8_t* ptr = tx.getSlot(i);
        *ptr = data[i];
    }

    ASSERT_TRUE(this->mQueue->commitWrite(dataLen));

    uint8_t readData[dataLen] = {};

    ASSERT_TRUE(this->mQueue->beginRead(dataLen, &tx));

    first = tx.getFirstRegion();
    second = tx.getSecondRegion();

    ASSERT_EQ(first.getLength() + second.getLength(),  dataLen);

    for (size_t i = 0; i < dataLen; i++) {
        uint8_t* ptr = tx.getSlot(i);
        readData[i] = *ptr;
    }

    ASSERT_TRUE(this->mQueue->commitRead(dataLen));

    ASSERT_EQ(0, memcmp(data, readData, dataLen));
}

/*
 * Verify that read() returns false when trying to read from an empty queue.
 */
TYPED_TEST(SynchronizedReadWrites, ReadWhenEmpty1) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    const size_t dataLen = 2;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    uint8_t readData[dataLen];
    ASSERT_FALSE(this->mQueue->read(readData, dataLen));
}

/*
 * Verify that beginRead() returns a MemTransaction object with null pointers when trying
 * to read from an empty queue.
 */
TYPED_TEST(SynchronizedReadWrites, ReadWhenEmpty2) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    const size_t dataLen = 2;
    ASSERT_LE(dataLen, this->mNumMessagesMax);

    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_FALSE(this->mQueue->beginRead(dataLen, &tx));

    auto first = tx.getFirstRegion();
    auto second = tx.getSecondRegion();

    ASSERT_EQ(nullptr, first.getAddress());
    ASSERT_EQ(nullptr, second.getAddress());
}

/*
 * Write the queue until full. Verify that another write is unsuccessful.
 * Verify that availableToWrite() returns 0 as expected.
 */
TYPED_TEST(SynchronizedReadWrites, WriteWhenFull1) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    std::vector<uint8_t> data(this->mNumMessagesMax);

    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    ASSERT_EQ(0UL, this->mQueue->availableToWrite());
    ASSERT_FALSE(this->mQueue->write(&data[0], 1));

    std::vector<uint8_t> readData(this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
    ASSERT_EQ(data, readData);
}

/*
 * Write the queue until full. Verify that beginWrite() returns
 * a MemTransaction object with null base pointers.
 */
TYPED_TEST(SynchronizedReadWrites, WriteWhenFull2) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    std::vector<uint8_t> data(this->mNumMessagesMax);

    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    ASSERT_EQ(0UL, this->mQueue->availableToWrite());

    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_FALSE(this->mQueue->beginWrite(1, &tx));

    auto first = tx.getFirstRegion();
    auto second = tx.getSecondRegion();

    ASSERT_EQ(nullptr, first.getAddress());
    ASSERT_EQ(nullptr, second.getAddress());
}

/*
 * Write a chunk of data equal to the queue size.
 * Verify that the write is successful and the subsequent read
 * returns the expected data.
 */
TYPED_TEST(SynchronizedReadWrites, LargeInputTest1) {
    std::vector<uint8_t> data(this->mNumMessagesMax);
    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    std::vector<uint8_t> readData(this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
    ASSERT_EQ(data, readData);
}

/*
 * Attempt to write a chunk of data larger than the queue size.
 * Verify that it fails. Verify that a subsequent read fails and
 * the queue is still empty.
 */
TYPED_TEST(SynchronizedReadWrites, LargeInputTest2) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    const size_t dataLen = 4096;
    ASSERT_GT(dataLen, this->mNumMessagesMax);
    std::vector<uint8_t> data(dataLen);

    initData(&data[0], dataLen);
    ASSERT_FALSE(this->mQueue->write(&data[0], dataLen));
    std::vector<uint8_t> readData(this->mNumMessagesMax);
    ASSERT_FALSE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
    ASSERT_NE(data, readData);
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
}

/*
 * After the queue is full, try to write more data. Verify that
 * the attempt returns false. Verify that the attempt did not
 * affect the pre-existing data in the queue.
 */
TYPED_TEST(SynchronizedReadWrites, LargeInputTest3) {
    std::vector<uint8_t> data(this->mNumMessagesMax);
    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    ASSERT_FALSE(this->mQueue->write(&data[0], 1));
    std::vector<uint8_t> readData(this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
    ASSERT_EQ(data, readData);
}

/*
 * Verify that beginWrite() returns a MemTransaction with
 * null base pointers when attempting to write data larger
 * than the queue size.
 */
TYPED_TEST(SynchronizedReadWrites, LargeInputTest4) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    const size_t dataLen = 4096;
    ASSERT_GT(dataLen, this->mNumMessagesMax);

    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_FALSE(this->mQueue->beginWrite(dataLen, &tx));

    auto first = tx.getFirstRegion();
    auto second = tx.getSecondRegion();

    ASSERT_EQ(nullptr, first.getAddress());
    ASSERT_EQ(nullptr, second.getAddress());
}

/*
 * Verify that multiple reads one after the other return expected data.
 */
TYPED_TEST(SynchronizedReadWrites, MultipleRead) {
    const size_t chunkSize = 100;
    const size_t chunkNum = 5;
    const size_t dataLen = chunkSize * chunkNum;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    uint8_t data[dataLen];

    initData(data, dataLen);
    ASSERT_TRUE(this->mQueue->write(data, dataLen));
    uint8_t readData[dataLen] = {};
    for (size_t i = 0; i < chunkNum; i++) {
        ASSERT_TRUE(this->mQueue->read(readData + i * chunkSize, chunkSize));
    }
    ASSERT_EQ(0, memcmp(readData, data, dataLen));
}

/*
 * Verify that multiple writes one after the other happens correctly.
 */
TYPED_TEST(SynchronizedReadWrites, MultipleWrite) {
    const int chunkSize = 100;
    const int chunkNum = 5;
    const size_t dataLen = chunkSize * chunkNum;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    uint8_t data[dataLen];

    initData(data, dataLen);
    for (unsigned int i = 0; i < chunkNum; i++) {
        ASSERT_TRUE(this->mQueue->write(data + i * chunkSize, chunkSize));
    }
    uint8_t readData[dataLen] = {};
    ASSERT_TRUE(this->mQueue->read(readData, dataLen));
    ASSERT_EQ(0, memcmp(readData, data, dataLen));
}

/*
 * Write enough messages into the FMQ to fill half of it
 * and read back the same.
 * Write this->mNumMessagesMax messages into the queue. This will cause a
 * wrap around. Read and verify the data.
 */
TYPED_TEST(SynchronizedReadWrites, ReadWriteWrapAround1) {
    size_t numMessages = this->mNumMessagesMax - 1;
    std::vector<uint8_t> data(this->mNumMessagesMax);
    std::vector<uint8_t> readData(this->mNumMessagesMax);
    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], numMessages));
    ASSERT_TRUE(this->mQueue->read(&readData[0], numMessages));
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    ASSERT_TRUE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
    ASSERT_EQ(data, readData);
}

/*
 * Use beginRead/CommitRead/beginWrite/commitWrite APIs
 * to test wrap arounds are handled correctly.
 * Write enough messages into the FMQ to fill half of it
 * and read back the same.
 * Write mNumMessagesMax messages into the queue. This will cause a
 * wrap around. Read and verify the data.
 */
TYPED_TEST(SynchronizedReadWrites, ReadWriteWrapAround2) {
    size_t dataLen = this->mNumMessagesMax - 1;
    std::vector<uint8_t> data(this->mNumMessagesMax);
    std::vector<uint8_t> readData(this->mNumMessagesMax);
    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], dataLen));
    ASSERT_TRUE(this->mQueue->read(&readData[0], dataLen));

    /*
     * The next write and read will have to deal with with wrap arounds.
     */
    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_TRUE(this->mQueue->beginWrite(this->mNumMessagesMax, &tx));

    auto first = tx.getFirstRegion();
    auto second = tx.getSecondRegion();

    ASSERT_EQ(first.getLength() + second.getLength(), this->mNumMessagesMax);

    ASSERT_TRUE(tx.copyTo(&data[0], 0 /* startIdx */, this->mNumMessagesMax));

    ASSERT_TRUE(this->mQueue->commitWrite(this->mNumMessagesMax));

    ASSERT_TRUE(this->mQueue->beginRead(this->mNumMessagesMax, &tx));

    first = tx.getFirstRegion();
    second = tx.getSecondRegion();

    ASSERT_EQ(first.getLength() + second.getLength(), this->mNumMessagesMax);

    ASSERT_TRUE(tx.copyFrom(&readData[0], 0 /* startIdx */, this->mNumMessagesMax));
    ASSERT_TRUE(this->mQueue->commitRead(this->mNumMessagesMax));

    ASSERT_EQ(data, readData);
}

/*
 * Verify that a few bytes of data can be successfully written and read.
 */
TYPED_TEST(UnsynchronizedWriteTest, SmallInputTest1) {
    const size_t dataLen = 16;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    uint8_t data[dataLen];

    initData(data, dataLen);
    ASSERT_TRUE(this->mQueue->write(data, dataLen));
    uint8_t readData[dataLen] = {};
    ASSERT_TRUE(this->mQueue->read(readData, dataLen));
    ASSERT_EQ(0, memcmp(data, readData, dataLen));
}

/*
 * Verify that read() returns false when trying to read from an empty queue.
 */
TYPED_TEST(UnsynchronizedWriteTest, ReadWhenEmpty) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    const size_t dataLen = 2;
    ASSERT_TRUE(dataLen < this->mNumMessagesMax);
    uint8_t readData[dataLen];
    ASSERT_FALSE(this->mQueue->read(readData, dataLen));
}

/*
 * Write the queue when full. Verify that a subsequent writes is succesful.
 * Verify that availableToWrite() returns 0 as expected.
 */
TYPED_TEST(UnsynchronizedWriteTest, WriteWhenFull1) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    std::vector<uint8_t> data(this->mNumMessagesMax);

    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    ASSERT_EQ(0UL, this->mQueue->availableToWrite());
    ASSERT_TRUE(this->mQueue->write(&data[0], 1));

    std::vector<uint8_t> readData(this->mNumMessagesMax);
    ASSERT_FALSE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
}

/*
 * Write the queue when full. Verify that a subsequent writes
 * using beginRead()/commitRead() is succesful.
 * Verify that the next read fails as expected for unsynchronized flavor.
 */
TYPED_TEST(UnsynchronizedWriteTest, WriteWhenFull2) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    std::vector<uint8_t> data(this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));

    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_TRUE(this->mQueue->beginWrite(1, &tx));

    ASSERT_EQ(tx.getFirstRegion().getLength(), 1U);

    ASSERT_TRUE(tx.copyTo(&data[0], 0 /* startIdx */));

    ASSERT_TRUE(this->mQueue->commitWrite(1));

    std::vector<uint8_t> readData(this->mNumMessagesMax);
    ASSERT_FALSE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
}

/*
 * Write a chunk of data equal to the queue size.
 * Verify that the write is successful and the subsequent read
 * returns the expected data.
 */
TYPED_TEST(UnsynchronizedWriteTest, LargeInputTest1) {
    std::vector<uint8_t> data(this->mNumMessagesMax);
    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    std::vector<uint8_t> readData(this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
    ASSERT_EQ(data, readData);
}

/*
 * Attempt to write a chunk of data larger than the queue size.
 * Verify that it fails. Verify that a subsequent read fails and
 * the queue is still empty.
 */
TYPED_TEST(UnsynchronizedWriteTest, LargeInputTest2) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    const size_t dataLen = 4096;
    ASSERT_GT(dataLen, this->mNumMessagesMax);
    std::vector<uint8_t> data(dataLen);
    initData(&data[0], dataLen);
    ASSERT_FALSE(this->mQueue->write(&data[0], dataLen));
    std::vector<uint8_t> readData(this->mNumMessagesMax);
    ASSERT_FALSE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
    ASSERT_NE(data, readData);
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
}

/*
 * After the queue is full, try to write more data. Verify that
 * the attempt is succesful. Verify that the read fails
 * as expected.
 */
TYPED_TEST(UnsynchronizedWriteTest, LargeInputTest3) {
    std::vector<uint8_t> data(this->mNumMessagesMax);
    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    ASSERT_TRUE(this->mQueue->write(&data[0], 1));
    std::vector<uint8_t> readData(this->mNumMessagesMax);
    ASSERT_FALSE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
}

/*
 * Verify that multiple reads one after the other return expected data.
 */
TYPED_TEST(UnsynchronizedWriteTest, MultipleRead) {
    const size_t chunkSize = 100;
    const size_t chunkNum = 5;
    const size_t dataLen = chunkSize * chunkNum;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    uint8_t data[dataLen];
    initData(data, dataLen);
    ASSERT_TRUE(this->mQueue->write(data, dataLen));
    uint8_t readData[dataLen] = {};
    for (size_t i = 0; i < chunkNum; i++) {
        ASSERT_TRUE(this->mQueue->read(readData + i * chunkSize, chunkSize));
    }
    ASSERT_EQ(0, memcmp(readData, data, dataLen));
}

/*
 * Verify that multiple writes one after the other happens correctly.
 */
TYPED_TEST(UnsynchronizedWriteTest, MultipleWrite) {
    const size_t chunkSize = 100;
    const size_t chunkNum = 5;
    const size_t dataLen = chunkSize * chunkNum;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    uint8_t data[dataLen];

    initData(data, dataLen);
    for (size_t i = 0; i < chunkNum; i++) {
        ASSERT_TRUE(this->mQueue->write(data + i * chunkSize, chunkSize));
    }

    uint8_t readData[dataLen] = {};
    ASSERT_TRUE(this->mQueue->read(readData, dataLen));
    ASSERT_EQ(0, memcmp(readData, data, dataLen));
}

/*
 * Write enough messages into the FMQ to fill half of it
 * and read back the same.
 * Write mNumMessagesMax messages into the queue. This will cause a
 * wrap around. Read and verify the data.
 */
TYPED_TEST(UnsynchronizedWriteTest, ReadWriteWrapAround) {
    size_t numMessages = this->mNumMessagesMax - 1;
    std::vector<uint8_t> data(this->mNumMessagesMax);
    std::vector<uint8_t> readData(this->mNumMessagesMax);

    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], numMessages));
    ASSERT_TRUE(this->mQueue->read(&readData[0], numMessages));
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    ASSERT_TRUE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
    ASSERT_EQ(data, readData);
}
