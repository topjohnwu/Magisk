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

#include <gtest/gtest.h>
#ifndef GTEST_IS_THREADSAFE
#error "GTest did not detect pthread library."
#endif

#include <aidl/android/fmq/test/FixedParcelable.h>
#include <aidl/android/fmq/test/ITestAidlMsgQ.h>
#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <android/hardware/tests/msgq/1.0/ITestMsgQ.h>
#include <fmq/AidlMessageQueue.h>
#include <fmq/EventFlag.h>
#include <fmq/MessageQueue.h>
#include <hidl/ServiceManagement.h>

// libutils:
using android::OK;
using android::sp;
using android::status_t;

// generated
using ::aidl::android::fmq::test::EventFlagBits;
using ::aidl::android::fmq::test::FixedParcelable;
using ::aidl::android::fmq::test::ITestAidlMsgQ;
using android::hardware::tests::msgq::V1_0::ITestMsgQ;

// libhidl
using android::hardware::kSynchronizedReadWrite;
using android::hardware::kUnsynchronizedWrite;
using android::hardware::MessageQueue;
using android::hardware::MQDescriptorSync;
using android::hardware::MQDescriptorUnsync;
using android::hardware::details::waitForHwService;

using aidl::android::hardware::common::fmq::SynchronizedReadWrite;
using aidl::android::hardware::common::fmq::UnsynchronizedWrite;
using android::hardware::kSynchronizedReadWrite;
using android::hardware::kUnsynchronizedWrite;

typedef android::AidlMessageQueue<int32_t, SynchronizedReadWrite> AidlMessageQueueSync;
typedef android::AidlMessageQueue<int32_t, UnsynchronizedWrite> AidlMessageQueueUnsync;
typedef android::hardware::MessageQueue<int32_t, kSynchronizedReadWrite> MessageQueueSync;
typedef android::hardware::MessageQueue<int32_t, kUnsynchronizedWrite> MessageQueueUnsync;
static const std::string kServiceName = "BnTestAidlMsgQ";
static constexpr size_t kNumElementsInSyncQueue = (PAGE_SIZE - 16) / sizeof(int32_t);

enum class SetupType {
    SINGLE_FD,
    DOUBLE_FD,
};

template <typename T, SetupType setupType>
class TestParamTypes {
  public:
    typedef T MQType;
    static constexpr bool UserFd = setupType == SetupType::DOUBLE_FD;
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

template <typename T>
class ClientSyncTestBase : public ::testing::Test {};

// Specialize for AIDL
template <>
class ClientSyncTestBase<AidlMessageQueueSync> : public ::testing::Test {
  protected:
    static std::shared_ptr<ITestAidlMsgQ> waitGetTestService() {
        const std::string instance = std::string() + ITestAidlMsgQ::descriptor + "/default";
        ndk::SpAIBinder binder(AServiceManager_getService(instance.c_str()));
        return ITestAidlMsgQ::fromBinder(binder);
    }
    bool configureFmqSyncReadWrite(AidlMessageQueueSync* mq) {
        bool result = false;
        auto ret = mService->configureFmqSyncReadWrite(mq->dupeDesc(), &result);
        return result && ret.isOk();
    }
    bool requestReadFmqSync(size_t dataLen) {
        bool result = false;
        auto ret = mService->requestReadFmqSync(dataLen, &result);
        return result && ret.isOk();
    }
    bool requestWriteFmqSync(size_t dataLen) {
        bool result = false;
        auto ret = mService->requestWriteFmqSync(dataLen, &result);
        return result && ret.isOk();
    }

    std::shared_ptr<ITestAidlMsgQ> mService;
};

// Specialize for HIDL
template <>
class ClientSyncTestBase<MessageQueueSync> : public ::testing::Test {
  protected:
    static sp<ITestMsgQ> waitGetTestService() {
        android::hardware::details::setTrebleTestingOverride(true);
        // waitForHwService is required because ITestMsgQ is not in manifest.xml.
        // "Real" HALs shouldn't be doing this.
        waitForHwService(ITestMsgQ::descriptor, "default");
        return ITestMsgQ::getService();
    }
    bool configureFmqSyncReadWrite(MessageQueueSync* mq) {
        auto ret = mService->configureFmqSyncReadWrite(*mq->getDesc());
        return ret && ret.isOk();
    }
    bool requestReadFmqSync(size_t dataLen) {
        auto ret = mService->requestReadFmqSync(dataLen);
        return ret && ret.isOk();
    }
    bool requestWriteFmqSync(size_t dataLen) {
        auto ret = mService->requestWriteFmqSync(dataLen);
        return ret && ret.isOk();
    }

    sp<ITestMsgQ> mService;
};

template <typename T>
class ClientUnsyncTestBase : public ::testing::Test {};

// Specialize for AIDL
template <>
class ClientUnsyncTestBase<AidlMessageQueueUnsync> : public ::testing::Test {
  protected:
    static std::shared_ptr<ITestAidlMsgQ> waitGetTestService() {
        const std::string instance = std::string() + ITestAidlMsgQ::descriptor + "/default";
        ndk::SpAIBinder binder(AServiceManager_getService(instance.c_str()));
        return ITestAidlMsgQ::fromBinder(binder);
    }
    bool getFmqUnsyncWrite(bool configureFmq, bool userFd, std::shared_ptr<ITestAidlMsgQ> service,
                           AidlMessageQueueUnsync** queue) {
        bool result = false;
        aidl::android::hardware::common::fmq::MQDescriptor<int32_t, UnsynchronizedWrite> desc;
        auto ret = service->getFmqUnsyncWrite(configureFmq, userFd, &desc, &result);
        *queue = new (std::nothrow) AidlMessageQueueUnsync(desc);
        return result && ret.isOk();
    }

    std::shared_ptr<ITestAidlMsgQ> getQueue(AidlMessageQueueUnsync** fmq, bool setupQueue,
                                            bool userFd) {
        std::shared_ptr<ITestAidlMsgQ> service = waitGetTestService();
        if (service == nullptr) return nullptr;
        getFmqUnsyncWrite(setupQueue, userFd, service, fmq);
        return service;
    }

    bool requestReadFmqUnsync(size_t dataLen, std::shared_ptr<ITestAidlMsgQ> service) {
        bool result = false;
        auto ret = service->requestReadFmqUnsync(dataLen, &result);
        return result && ret.isOk();
    }
    bool requestWriteFmqUnsync(size_t dataLen, std::shared_ptr<ITestAidlMsgQ> service) {
        bool result = false;
        auto ret = service->requestWriteFmqUnsync(dataLen, &result);
        return result && ret.isOk();
    }
    AidlMessageQueueUnsync* newQueue() {
        if (mQueue->isValid())
            return new (std::nothrow) AidlMessageQueueUnsync(mQueue->dupeDesc());
        else
            return nullptr;
    }

    std::shared_ptr<ITestAidlMsgQ> mService;
    AidlMessageQueueUnsync* mQueue = nullptr;
};

// Specialize for HIDL
template <>
class ClientUnsyncTestBase<MessageQueueUnsync> : public ::testing::Test {
  protected:
    static sp<ITestMsgQ> waitGetTestService() {
        android::hardware::details::setTrebleTestingOverride(true);
        // waitForHwService is required because ITestMsgQ is not in manifest.xml.
        // "Real" HALs shouldn't be doing this.
        waitForHwService(ITestMsgQ::descriptor, "default");
        return ITestMsgQ::getService();
    }
    bool getFmqUnsyncWrite(bool configureFmq, bool userFd, sp<ITestMsgQ> service,
                           MessageQueueUnsync** queue) {
        if (!service) {
            return false;
        }
        service->getFmqUnsyncWrite(configureFmq, userFd,
                                   [queue](bool ret, const MQDescriptorUnsync<int32_t>& in) {
                                       ASSERT_TRUE(ret);
                                       *queue = new (std::nothrow) MessageQueueUnsync(in);
                                   });
        return true;
    }

    sp<ITestMsgQ> getQueue(MessageQueueUnsync** fmq, bool setupQueue, bool userFd) {
        sp<ITestMsgQ> service = waitGetTestService();
        if (service == nullptr) return nullptr;
        getFmqUnsyncWrite(setupQueue, userFd, service, fmq);
        return service;
    }

    bool requestReadFmqUnsync(size_t dataLen, sp<ITestMsgQ> service) {
        auto ret = service->requestReadFmqUnsync(dataLen);
        return ret && ret.isOk();
    }
    bool requestWriteFmqUnsync(size_t dataLen, sp<ITestMsgQ> service) {
        auto ret = service->requestWriteFmqUnsync(dataLen);
        return ret && ret.isOk();
    }

    MessageQueueUnsync* newQueue() {
        return new (std::nothrow) MessageQueueUnsync(*mQueue->getDesc());
    }

    sp<ITestMsgQ> mService;
    MessageQueueUnsync* mQueue = nullptr;
};

TYPED_TEST_CASE(UnsynchronizedWriteClientMultiProcess, UnsyncTypes);
template <typename T>
class UnsynchronizedWriteClientMultiProcess : public ClientUnsyncTestBase<typename T::MQType> {};

TYPED_TEST_CASE(SynchronizedReadWriteClient, SyncTypes);
template <typename T>
class SynchronizedReadWriteClient : public ClientSyncTestBase<typename T::MQType> {
  protected:
    virtual void TearDown() {
        delete mQueue;
    }

    virtual void SetUp() {
        this->mService = this->waitGetTestService();
        ASSERT_NE(this->mService, nullptr);
        ASSERT_TRUE(this->mService->isRemote());
        static constexpr size_t kSyncElementSizeBytes = sizeof(int32_t);
        android::base::unique_fd ringbufferFd;
        if (T::UserFd) {
            ringbufferFd.reset(::ashmem_create_region(
                    "SyncReadWrite", kNumElementsInSyncQueue * kSyncElementSizeBytes));
        }
        // create a queue on the client side
        mQueue = new (std::nothrow) typename T::MQType(
                kNumElementsInSyncQueue, true /* configure event flag word */,
                std::move(ringbufferFd), kNumElementsInSyncQueue * kSyncElementSizeBytes);
        ASSERT_NE(nullptr, mQueue);
        ASSERT_TRUE(mQueue->isValid());
        ASSERT_EQ(mQueue->getQuantumCount(), kNumElementsInSyncQueue);

        // tell server to set up the queue on its end
        ASSERT_TRUE(this->configureFmqSyncReadWrite(mQueue));
    }

    typename T::MQType* mQueue = nullptr;
};

TYPED_TEST_CASE(UnsynchronizedWriteClient, UnsyncTypes);
template <typename T>
class UnsynchronizedWriteClient : public ClientUnsyncTestBase<typename T::MQType> {
  protected:
    virtual void TearDown() { delete this->mQueue; }

    virtual void SetUp() {
        this->mService = this->waitGetTestService();
        ASSERT_NE(this->mService, nullptr);
        ASSERT_TRUE(this->mService->isRemote());
        this->getFmqUnsyncWrite(true, false, this->mService, &this->mQueue);
        ASSERT_NE(nullptr, this->mQueue);
        ASSERT_TRUE(this->mQueue->isValid());
        mNumMessagesMax = this->mQueue->getQuantumCount();
    }

    size_t mNumMessagesMax = 0;
};

/*
 * Utility function to verify data read from the fast message queue.
 */
bool verifyData(int32_t* data, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (data[i] != i) return false;
    }
    return true;
}

/*
 * Utility function to initialize data to be written to the FMQ
 */
inline void initData(int32_t* data, size_t count) {
    for (size_t i = 0; i < count; i++) {
        data[i] = i;
    }
}

/*
 * Verify that for an unsynchronized flavor of FMQ, multiple readers
 * can recover from a write overflow condition.
 */
TYPED_TEST(UnsynchronizedWriteClientMultiProcess, MultipleReadersAfterOverflow) {
    const size_t dataLen = 16;

    pid_t pid;
    /* creating first reader process */
    if ((pid = fork()) == 0) {
        typename TypeParam::MQType* queue = nullptr;
        auto service =
                this->getQueue(&queue, true /* setupQueue */, TypeParam::UserFd /* userFd */);
        ASSERT_NE(service, nullptr);
        ASSERT_TRUE(service->isRemote());
        ASSERT_NE(queue, nullptr);
        ASSERT_TRUE(queue->isValid());

        size_t numMessagesMax = queue->getQuantumCount();

        // The following two writes will cause a write overflow.
        auto ret = this->requestWriteFmqUnsync(numMessagesMax, service);
        ASSERT_TRUE(ret);

        ret = this->requestWriteFmqUnsync(1, service);
        ASSERT_TRUE(ret);

        // The following read should fail due to the overflow.
        std::vector<int32_t> readData(numMessagesMax);
        ASSERT_FALSE(queue->read(&readData[0], numMessagesMax));

        /*
         * Request another write to verify that the reader can recover from the
         * overflow condition.
         */
        ASSERT_LT(dataLen, numMessagesMax);
        ret = this->requestWriteFmqUnsync(dataLen, service);
        ASSERT_TRUE(ret);

        // Verify that the read is successful.
        ASSERT_TRUE(queue->read(&readData[0], dataLen));
        ASSERT_TRUE(verifyData(&readData[0], dataLen));

        delete queue;
        exit(0);
    }

    ASSERT_GT(pid, 0 /* parent should see PID greater than 0 for a good fork */);

    int status;
    // wait for the first reader process to exit.
    ASSERT_EQ(pid, waitpid(pid, &status, 0 /* options */));

    // creating second reader process.
    if ((pid = fork()) == 0) {
        typename TypeParam::MQType* queue = nullptr;
        auto service = this->getQueue(&queue, false /* setupQueue */, false /* userFd */);
        ASSERT_NE(service, nullptr);
        ASSERT_TRUE(service->isRemote());
        ASSERT_NE(queue, nullptr);
        ASSERT_TRUE(queue->isValid());

        // This read should fail due to the write overflow.
        std::vector<int32_t> readData(dataLen);
        ASSERT_FALSE(queue->read(&readData[0], dataLen));

        /*
         * Request another write to verify that the process that recover from
         * the overflow condition.
         */
        auto ret = this->requestWriteFmqUnsync(dataLen, service);
        ASSERT_TRUE(ret);

        // verify that the read is successful.
        ASSERT_TRUE(queue->read(&readData[0], dataLen));
        ASSERT_TRUE(verifyData(&readData[0], dataLen));

        delete queue;
        exit(0);
    }

    ASSERT_GT(pid, 0 /* parent should see PID greater than 0 for a good fork */);
    ASSERT_EQ(pid, waitpid(pid, &status, 0 /* options */));
}

/*
 * Test that basic blocking works using readBlocking()/writeBlocking() APIs
 * using the EventFlag object owned by FMQ.
 */
TYPED_TEST(SynchronizedReadWriteClient, BlockingReadWrite1) {
    const size_t dataLen = 64;
    bool ret = false;
    /*
     * Request service to perform a blocking read. This call is oneway and will
     * return immediately.
     */
    this->mService->requestBlockingRead(dataLen);
    {
        std::array<int32_t, dataLen> data = {0};
        ret = this->mQueue->writeBlocking(
                data.data(), data.size(),
                static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_EMPTY),
                5000000000 /* timeOutNanos */);
        ASSERT_TRUE(ret);
    }
    {
        std::array<int32_t, kNumElementsInSyncQueue> data = {0};
        ret = this->mQueue->writeBlocking(
                data.data(), data.size(),
                static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_EMPTY),
                5000000000 /* timeOutNanos */);
        ASSERT_TRUE(ret);
    }
}

/*
 * Test that basic blocking works using readBlocking()/writeBlocking() APIs
 * using the EventFlag object owned by FMQ and using the default EventFlag
 * notification bit mask.
 */
TYPED_TEST(SynchronizedReadWriteClient, BlockingReadWrite2) {
    const size_t dataLen = 64;
    bool ret = false;

    /*
     * Request service to perform a blocking read using default EventFlag
     * notification bit mask. This call is oneway and will
     * return immediately.
     */
    this->mService->requestBlockingReadDefaultEventFlagBits(dataLen);

    /* Cause a context switch to allow service to block */
    sched_yield();
    {
        std::array<int32_t, dataLen> data = {0};
        ret = this->mQueue->writeBlocking(data.data(), data.size());
        ASSERT_TRUE(ret);
    }

    /*
     * If the blocking read was successful, another write of size
     * kNumElementsInSyncQueue will succeed.
     */
    {
        std::array<int32_t, kNumElementsInSyncQueue> data = {0};
        ret = this->mQueue->writeBlocking(data.data(), data.size(), 5000000000 /* timeOutNanos */);
        ASSERT_TRUE(ret);
    }
}

/*
 * Test that repeated blocking reads and writes work using readBlocking()/writeBlocking() APIs
 * using the EventFlag object owned by FMQ.
 * Each write operation writes the same amount of data as a single read
 * operation.
 */
TYPED_TEST(SynchronizedReadWriteClient, BlockingReadWriteRepeat1) {
    const size_t dataLen = 64;
    bool ret = false;

    /*
     * Request service to perform a blocking read of 64 elements. This call is
     * oneway and will return immediately.
     */
    const size_t writeCount = kNumElementsInSyncQueue;
    this->mService->requestBlockingReadRepeat(dataLen, writeCount);
    /*
     * Write 64 elements into the queue for the service to consume
     */
    {
        std::array<int32_t, dataLen> data = {0};
        for (size_t i = 0; i < writeCount; i++) {
            ret = this->mQueue->writeBlocking(
                    data.data(), data.size(),
                    static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_FULL),
                    static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_EMPTY),
                    5000000000 /* timeOutNanos */);
            ASSERT_TRUE(ret);
        }
    }
    /*
     * The queue should be totally empty now, so filling it up entirely with one
     * blocking write should be successful.
     */
    {
        std::array<int32_t, kNumElementsInSyncQueue> data = {0};
        ret = this->mQueue->writeBlocking(
                data.data(), data.size(),
                static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_EMPTY),
                5000000000 /* timeOutNanos */);

        ASSERT_TRUE(ret);
    }
}

/*
 * Test that repeated blocking reads and writes work using readBlocking()/writeBlocking() APIs
 * using the EventFlag object owned by FMQ. Each read operation reads twice the
 * amount of data as a single write.
 *
 */
TYPED_TEST(SynchronizedReadWriteClient, BlockingReadWriteRepeat2) {
    const size_t dataLen = 64;
    bool ret = false;
    /*
     * Request service to perform a repeated blocking read. This call is oneway
     * and will return immediately. It will read 64 * 2 elements with each
     * blocking read, for a total of writeCount / 2 calls.
     */
    const size_t writeCount = kNumElementsInSyncQueue;
    this->mService->requestBlockingReadRepeat(dataLen * 2, writeCount / 2);
    /*
     * Write 64 elements into the queue writeCount times
     */
    {
        std::array<int32_t, dataLen> data = {0};
        for (size_t i = 0; i < writeCount; i++) {
            ret = this->mQueue->writeBlocking(
                    data.data(), data.size(),
                    static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_FULL),
                    static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_EMPTY),
                    5000000000 /* timeOutNanos */);
            ASSERT_TRUE(ret);
        }
    }
    /*
     * The queue should be totally empty now, so filling it up entirely with one
     * blocking write should be successful.
     */
    {
        std::array<int32_t, kNumElementsInSyncQueue> data = {0};
        ret = this->mQueue->writeBlocking(
                data.data(), data.size(),
                static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_EMPTY),
                5000000000 /* timeOutNanos */);
        ASSERT_TRUE(ret);
    }
}

/*
 * Test that basic blocking works using readBlocking()/writeBlocking() APIs
 * using the EventFlag object owned by FMQ. Each write operation writes twice
 * the amount of data as a single read.
 */
TYPED_TEST(SynchronizedReadWriteClient, BlockingReadWriteRepeat3) {
    const size_t dataLen = 64;
    bool ret = false;

    /*
     * Request service to perform a repeated blocking read. This call is oneway
     * and will return immediately. It will read 64 / 2 elements with each
     * blocking read, for a total of writeCount * 2 calls.
     */
    size_t writeCount = 1024;
    this->mService->requestBlockingReadRepeat(dataLen / 2, writeCount * 2);
    /*
     * Write 64 elements into the queue writeCount times
     */
    {
        std::array<int32_t, dataLen> data = {0};
        for (size_t i = 0; i < writeCount; i++) {
            ret = this->mQueue->writeBlocking(
                    data.data(), data.size(),
                    static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_FULL),
                    static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_EMPTY),
                    5000000000 /* timeOutNanos */);
            ASSERT_TRUE(ret);
        }
    }
    /*
     * The queue should be totally empty now, so filling it up entirely with one
     * blocking write should be successful.
     */
    {
        std::array<int32_t, kNumElementsInSyncQueue> data = {0};
        ret = this->mQueue->writeBlocking(
                data.data(), data.size(),
                static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_EMPTY),
                5000000000 /* timeOutNanos */);
        ASSERT_TRUE(ret);
    }
}

/*
 * Test that writeBlocking()/readBlocking() APIs do not block on
 * attempts to write/read 0 messages and return true.
 */
TYPED_TEST(SynchronizedReadWriteClient, BlockingReadWriteZeroMessages) {
    int32_t data = 0;

    /*
     * Trigger a blocking write for zero messages with no timeout.
     */
    bool ret = this->mQueue->writeBlocking(
            &data, 0, static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_FULL),
            static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_EMPTY));
    ASSERT_TRUE(ret);

    /*
     * Trigger a blocking read for zero messages with no timeout.
     */
    ret = this->mQueue->readBlocking(
            &data, 0, static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_FULL),
            static_cast<uint32_t>(ITestMsgQ::EventFlagBits::FMQ_NOT_EMPTY));
    ASSERT_TRUE(ret);
}

/*
 * Request mService to write a small number of messages
 * to the FMQ. Read and verify data.
 */
TYPED_TEST(SynchronizedReadWriteClient, SmallInputReaderTest1) {
    const size_t dataLen = 16;
    ASSERT_LE(dataLen, kNumElementsInSyncQueue);
    bool ret = this->requestWriteFmqSync(dataLen);
    ASSERT_TRUE(ret);
    int32_t readData[dataLen] = {};
    ASSERT_TRUE(this->mQueue->read(readData, dataLen));
    ASSERT_TRUE(verifyData(readData, dataLen));
}

/*
 * Request mService to write a message to the queue followed by a beginRead().
 * Get a pointer to the memory region for the that first message. Set the write
 * counter to the last byte in the ring buffer. Request another write from
 * mService. The write should fail because the write address is misaligned.
 */
TYPED_TEST(SynchronizedReadWriteClient, MisalignedWriteCounter) {
    if (TypeParam::UserFd) {
        // When using the second FD for the ring buffer, we can't get to the read/write
        // counters from a pointer to the ring buffer, so no sense in testing.
        GTEST_SKIP();
    }
    const size_t dataLen = 1;
    ASSERT_LE(dataLen, kNumElementsInSyncQueue);
    bool ret = this->requestWriteFmqSync(dataLen);
    ASSERT_TRUE(ret);
    // begin read and get a MemTransaction object for the first object in the queue
    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_TRUE(this->mQueue->beginRead(dataLen, &tx));
    // get a pointer to the beginning of the ring buffer
    const auto& region = tx.getFirstRegion();
    int32_t* firstStart = region.getAddress();

    // because this is the first location in the ring buffer, we can get
    // access to the read and write pointer stored in the fd. 8 bytes back for the
    // write counter and 16 bytes back for the read counter
    uint64_t* writeCntr = (uint64_t*)((uint8_t*)firstStart - 8);

    // set it to point to the very last byte in the ring buffer
    *(writeCntr) = this->mQueue->getQuantumCount() * this->mQueue->getQuantumSize() - 1;
    ASSERT_TRUE(*writeCntr % sizeof(int32_t) != 0);

    // this is not actually necessary, but it's the expected the pattern.
    this->mQueue->commitRead(dataLen);

    // This next write will be misaligned and will overlap outside of the ring buffer.
    // The write should fail.
    ret = this->requestWriteFmqSync(dataLen);
    EXPECT_FALSE(ret);
}

/*
 * Request mService to write a small number of messages
 * to the FMQ. Read and verify each message using
 * beginRead/Commit read APIs.
 */
TYPED_TEST(SynchronizedReadWriteClient, SmallInputReaderTest2) {
    const size_t dataLen = 16;
    ASSERT_LE(dataLen, kNumElementsInSyncQueue);
    auto ret = this->requestWriteFmqSync(dataLen);
    ASSERT_TRUE(ret);

    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_TRUE(this->mQueue->beginRead(dataLen, &tx));

    auto first = tx.getFirstRegion();
    auto second = tx.getSecondRegion();
    size_t firstRegionLength = first.getLength();

    for (size_t i = 0; i < dataLen; i++) {
        if (i < firstRegionLength) {
            ASSERT_EQ(i, *(first.getAddress() + i));
        } else {
            ASSERT_EQ(i, *(second.getAddress() + i - firstRegionLength));
        }
    }

    ASSERT_TRUE(this->mQueue->commitRead(dataLen));
}

/*
 * Write a small number of messages to FMQ. Request
 * mService to read and verify that the write was successful.
 */
TYPED_TEST(SynchronizedReadWriteClient, SmallInputWriterTest1) {
    const size_t dataLen = 16;
    ASSERT_LE(dataLen, kNumElementsInSyncQueue);
    size_t originalCount = this->mQueue->availableToWrite();
    int32_t data[dataLen];
    initData(data, dataLen);
    ASSERT_TRUE(this->mQueue->write(data, dataLen));
    bool ret = this->requestReadFmqSync(dataLen);
    ASSERT_TRUE(ret);
    size_t availableCount = this->mQueue->availableToWrite();
    ASSERT_EQ(originalCount, availableCount);
}

/*
 * Write a small number of messages to FMQ using the beginWrite()/CommitWrite()
 * APIs. Request mService to read and verify that the write was successful.
 */
TYPED_TEST(SynchronizedReadWriteClient, SmallInputWriterTest2) {
    const size_t dataLen = 16;
    ASSERT_LE(dataLen, kNumElementsInSyncQueue);
    size_t originalCount = this->mQueue->availableToWrite();
    int32_t data[dataLen];
    initData(data, dataLen);

    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_TRUE(this->mQueue->beginWrite(dataLen, &tx));

    auto first = tx.getFirstRegion();
    auto second = tx.getSecondRegion();

    size_t firstRegionLength = first.getLength();
    int32_t* firstBaseAddress = first.getAddress();
    int32_t* secondBaseAddress = second.getAddress();

    for (size_t i = 0; i < dataLen; i++) {
        if (i < firstRegionLength) {
            *(firstBaseAddress + i) = i;
        } else {
            *(secondBaseAddress + i - firstRegionLength) = i;
        }
    }

    ASSERT_TRUE(this->mQueue->commitWrite(dataLen));

    auto ret = this->requestReadFmqSync(dataLen);
    // ASSERT_TRUE(ret.isOk());
    ASSERT_TRUE(ret);
    size_t availableCount = this->mQueue->availableToWrite();
    ASSERT_EQ(originalCount, availableCount);
}

/*
 * Verify that the FMQ is empty and read fails when it is empty.
 */
TYPED_TEST(SynchronizedReadWriteClient, ReadWhenEmpty) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    const size_t numMessages = 2;
    ASSERT_LE(numMessages, kNumElementsInSyncQueue);
    int32_t readData[numMessages];
    ASSERT_FALSE(this->mQueue->read(readData, numMessages));
}

/*
 * Verify FMQ is empty.
 * Write enough messages to fill it.
 * Verify availableToWrite() method returns is zero.
 * Try writing another message and verify that
 * the attempted write was unsuccessful. Request mService
 * to read and verify the messages in the FMQ.
 */
TYPED_TEST(SynchronizedReadWriteClient, WriteWhenFull) {
    std::array<int32_t, kNumElementsInSyncQueue> data = {0};
    initData(data.data(), data.size());
    ASSERT_TRUE(this->mQueue->write(data.data(), data.size()));
    ASSERT_EQ(0UL, this->mQueue->availableToWrite());
    ASSERT_FALSE(this->mQueue->write(&data[0], 1));
    bool ret = this->requestReadFmqSync(data.size());
    ASSERT_TRUE(ret);
}

/*
 * Verify FMQ is empty.
 * Request mService to write data equal to queue size.
 * Read and verify data in mQueue.
 */
TYPED_TEST(SynchronizedReadWriteClient, LargeInputTest1) {
    bool ret = this->requestWriteFmqSync(kNumElementsInSyncQueue);
    ASSERT_TRUE(ret);
    std::vector<int32_t> readData(kNumElementsInSyncQueue);
    ASSERT_TRUE(this->mQueue->read(&readData[0], kNumElementsInSyncQueue));
    ASSERT_TRUE(verifyData(&readData[0], kNumElementsInSyncQueue));
}

/*
 * Request mService to write more than maximum number of messages to the FMQ.
 * Verify that the write fails. Verify that availableToRead() method
 * still returns 0 and verify that attempt to read fails.
 */
TYPED_TEST(SynchronizedReadWriteClient, LargeInputTest2) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    const size_t numMessages = 2048;
    ASSERT_GT(numMessages, kNumElementsInSyncQueue);
    bool ret = this->requestWriteFmqSync(numMessages);
    ASSERT_FALSE(ret);
    int32_t readData;
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    ASSERT_FALSE(this->mQueue->read(&readData, 1));
}

/*
 * Write until FMQ is full.
 * Verify that the number of messages available to write
 * is equal to mNumMessagesMax.
 * Verify that another write attempt fails.
 * Request mService to read. Verify read count.
 */

TYPED_TEST(SynchronizedReadWriteClient, LargeInputTest3) {
    std::array<int32_t, kNumElementsInSyncQueue> data = {0};
    initData(data.data(), data.size());
    ASSERT_TRUE(this->mQueue->write(data.data(), data.size()));
    ASSERT_EQ(0UL, this->mQueue->availableToWrite());
    ASSERT_FALSE(this->mQueue->write(data.data(), 1));

    bool ret = this->requestReadFmqSync(data.size());
    ASSERT_TRUE(ret);
}

/*
 * Confirm that the FMQ is empty. Request mService to write to FMQ.
 * Do multiple reads to empty FMQ and verify data.
 */
TYPED_TEST(SynchronizedReadWriteClient, MultipleRead) {
    const size_t chunkSize = 100;
    const size_t chunkNum = 5;
    const size_t numMessages = chunkSize * chunkNum;
    ASSERT_LE(numMessages, kNumElementsInSyncQueue);
    size_t availableToRead = this->mQueue->availableToRead();
    size_t expectedCount = 0;
    ASSERT_EQ(expectedCount, availableToRead);
    bool ret = this->requestWriteFmqSync(numMessages);
    ASSERT_TRUE(ret);
    int32_t readData[numMessages] = {};
    for (size_t i = 0; i < chunkNum; i++) {
        ASSERT_TRUE(this->mQueue->read(readData + i * chunkSize, chunkSize));
    }
    ASSERT_TRUE(verifyData(readData, numMessages));
}

/*
 * Write to FMQ in bursts.
 * Request mService to read data. Verify the read was successful.
 */
TYPED_TEST(SynchronizedReadWriteClient, MultipleWrite) {
    const size_t chunkSize = 100;
    const size_t chunkNum = 5;
    const size_t numMessages = chunkSize * chunkNum;
    ASSERT_LE(numMessages, kNumElementsInSyncQueue);
    int32_t data[numMessages];
    initData(&data[0], numMessages);

    for (size_t i = 0; i < chunkNum; i++) {
        ASSERT_TRUE(this->mQueue->write(data + i * chunkSize, chunkSize));
    }
    bool ret = this->requestReadFmqSync(numMessages);
    ASSERT_TRUE(ret);
}

/*
 * Write enough messages into the FMQ to fill half of it.
 * Request mService to read back the same.
 * Write mNumMessagesMax messages into the queue. This should cause a
 * wrap around. Request mService to read and verify the data.
 */
TYPED_TEST(SynchronizedReadWriteClient, ReadWriteWrapAround) {
    size_t numMessages = kNumElementsInSyncQueue / 2;
    std::array<int32_t, kNumElementsInSyncQueue> data = {0};
    initData(data.data(), data.size());
    ASSERT_TRUE(this->mQueue->write(&data[0], numMessages));
    bool ret = this->requestReadFmqSync(numMessages);
    ASSERT_TRUE(ret);
    ASSERT_TRUE(this->mQueue->write(data.data(), data.size()));
    ret = this->requestReadFmqSync(data.size());
    ASSERT_TRUE(ret);
}

/*
 * Use beginWrite/commitWrite/getSlot APIs to test wrap arounds are handled
 * correctly.
 * Write enough messages into the FMQ to fill half of it
 * and read back the same.
 * Write mNumMessagesMax messages into the queue. This will cause a
 * wrap around. Read and verify the data.
 */
TYPED_TEST(SynchronizedReadWriteClient, ReadWriteWrapAround2) {
    size_t numMessages = kNumElementsInSyncQueue / 2;
    std::array<int32_t, kNumElementsInSyncQueue> data = {0};
    initData(data.data(), data.size());
    ASSERT_TRUE(this->mQueue->write(&data[0], numMessages));
    auto ret = this->requestReadFmqSync(numMessages);
    ASSERT_TRUE(ret);

    /*
     * The next write and read will have to deal with with wrap arounds.
     */
    typename TypeParam::MQType::MemTransaction tx;
    ASSERT_TRUE(this->mQueue->beginWrite(data.size(), &tx));

    ASSERT_EQ(tx.getFirstRegion().getLength() + tx.getSecondRegion().getLength(), data.size());

    for (size_t i = 0; i < data.size(); i++) {
        int32_t* ptr = tx.getSlot(i);
        *ptr = data[i];
    }

    ASSERT_TRUE(this->mQueue->commitWrite(data.size()));

    ret = this->requestReadFmqSync(data.size());
    ASSERT_TRUE(ret);
}

/*
 * Request this->mService to write a small number of messages
 * to the FMQ. Read and verify data.
 */
TYPED_TEST(UnsynchronizedWriteClient, SmallInputReaderTest1) {
    const size_t dataLen = 16;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    bool ret = this->requestWriteFmqUnsync(dataLen, this->mService);
    ASSERT_TRUE(ret);
    int32_t readData[dataLen] = {};
    ASSERT_TRUE(this->mQueue->read(readData, dataLen));
    ASSERT_TRUE(verifyData(readData, dataLen));
}

/*
 * Write a small number of messages to FMQ. Request
 * this->mService to read and verify that the write was successful.
 */
TYPED_TEST(UnsynchronizedWriteClient, SmallInputWriterTest1) {
    const size_t dataLen = 16;
    ASSERT_LE(dataLen, this->mNumMessagesMax);
    int32_t data[dataLen];
    initData(data, dataLen);
    ASSERT_TRUE(this->mQueue->write(data, dataLen));
    bool ret = this->requestReadFmqUnsync(dataLen, this->mService);
    ASSERT_TRUE(ret);
}

/*
 * Verify that the FMQ is empty and read fails when it is empty.
 */
TYPED_TEST(UnsynchronizedWriteClient, ReadWhenEmpty) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    const size_t numMessages = 2;
    ASSERT_LE(numMessages, this->mNumMessagesMax);
    int32_t readData[numMessages];
    ASSERT_FALSE(this->mQueue->read(readData, numMessages));
}

/*
 * Verify FMQ is empty.
 * Write enough messages to fill it.
 * Verify availableToWrite() method returns is zero.
 * Try writing another message and verify that
 * the attempted write was successful. Request this->mService
 * to read the messages in the FMQ and verify that it is unsuccessful.
 */

TYPED_TEST(UnsynchronizedWriteClient, WriteWhenFull) {
    std::vector<int32_t> data(this->mNumMessagesMax);
    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    ASSERT_EQ(0UL, this->mQueue->availableToWrite());
    ASSERT_TRUE(this->mQueue->write(&data[0], 1));
    bool ret = this->requestReadFmqUnsync(this->mNumMessagesMax, this->mService);
    ASSERT_FALSE(ret);
}

/*
 * Verify FMQ is empty.
 * Request this->mService to write data equal to queue size.
 * Read and verify data in this->mQueue.
 */
TYPED_TEST(UnsynchronizedWriteClient, LargeInputTest1) {
    bool ret = this->requestWriteFmqUnsync(this->mNumMessagesMax, this->mService);
    ASSERT_TRUE(ret);
    std::vector<int32_t> data(this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->read(&data[0], this->mNumMessagesMax));
    ASSERT_TRUE(verifyData(&data[0], this->mNumMessagesMax));
}

/*
 * Request this->mService to write more than maximum number of messages to the FMQ.
 * Verify that the write fails. Verify that availableToRead() method
 * still returns 0 and verify that attempt to read fails.
 */
TYPED_TEST(UnsynchronizedWriteClient, LargeInputTest2) {
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    const size_t numMessages = this->mNumMessagesMax + 1;
    bool ret = this->requestWriteFmqUnsync(numMessages, this->mService);
    ASSERT_FALSE(ret);
    int32_t readData;
    ASSERT_EQ(0UL, this->mQueue->availableToRead());
    ASSERT_FALSE(this->mQueue->read(&readData, 1));
}

/*
 * Write until FMQ is full.
 * Verify that the number of messages available to write
 * is equal to this->mNumMessagesMax.
 * Verify that another write attempt is successful.
 * Request this->mService to read. Verify that read is unsuccessful.
 * Perform another write and verify that the read is successful
 * to check if the reader process can recover from the error condition.
 */
TYPED_TEST(UnsynchronizedWriteClient, LargeInputTest3) {
    std::vector<int32_t> data(this->mNumMessagesMax);
    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    ASSERT_EQ(0UL, this->mQueue->availableToWrite());
    ASSERT_TRUE(this->mQueue->write(&data[0], 1));

    bool ret = this->requestReadFmqUnsync(this->mNumMessagesMax, this->mService);
    ASSERT_FALSE(ret);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));

    ret = this->requestReadFmqUnsync(this->mNumMessagesMax, this->mService);
    ASSERT_TRUE(ret);
}

/*
 * Confirm that the FMQ is empty. Request this->mService to write to FMQ.
 * Do multiple reads to empty FMQ and verify data.
 */
TYPED_TEST(UnsynchronizedWriteClient, MultipleRead) {
    const size_t chunkSize = 100;
    const size_t chunkNum = 5;
    const size_t numMessages = chunkSize * chunkNum;
    ASSERT_LE(numMessages, this->mNumMessagesMax);
    size_t availableToRead = this->mQueue->availableToRead();
    size_t expectedCount = 0;
    ASSERT_EQ(expectedCount, availableToRead);
    bool ret = this->requestWriteFmqUnsync(numMessages, this->mService);
    ASSERT_TRUE(ret);
    int32_t readData[numMessages] = {};
    for (size_t i = 0; i < chunkNum; i++) {
        ASSERT_TRUE(this->mQueue->read(readData + i * chunkSize, chunkSize));
    }
    ASSERT_TRUE(verifyData(readData, numMessages));
}

/*
 * Write to FMQ in bursts.
 * Request this->mService to read data, verify that it was successful.
 */
TYPED_TEST(UnsynchronizedWriteClient, MultipleWrite) {
    const size_t chunkSize = 100;
    const size_t chunkNum = 5;
    const size_t numMessages = chunkSize * chunkNum;
    ASSERT_LE(numMessages, this->mNumMessagesMax);
    int32_t data[numMessages];
    initData(data, numMessages);
    for (size_t i = 0; i < chunkNum; i++) {
        ASSERT_TRUE(this->mQueue->write(data + i * chunkSize, chunkSize));
    }
    bool ret = this->requestReadFmqUnsync(numMessages, this->mService);
    ASSERT_TRUE(ret);
}

/*
 * Write enough messages into the FMQ to fill half of it.
 * Request this->mService to read back the same.
 * Write this->mNumMessagesMax messages into the queue. This should cause a
 * wrap around. Request this->mService to read and verify the data.
 */
TYPED_TEST(UnsynchronizedWriteClient, ReadWriteWrapAround) {
    size_t numMessages = this->mNumMessagesMax / 2;
    std::vector<int32_t> data(this->mNumMessagesMax);
    initData(&data[0], this->mNumMessagesMax);
    ASSERT_TRUE(this->mQueue->write(&data[0], numMessages));
    bool ret = this->requestReadFmqUnsync(numMessages, this->mService);
    ASSERT_TRUE(ret);
    ASSERT_TRUE(this->mQueue->write(&data[0], this->mNumMessagesMax));
    ret = this->requestReadFmqUnsync(this->mNumMessagesMax, this->mService);
    ASSERT_TRUE(ret);
}

/*
 * Request this->mService to write a small number of messages
 * to the FMQ. Read and verify data from two threads configured
 * as readers to the FMQ.
 */
TYPED_TEST(UnsynchronizedWriteClient, SmallInputMultipleReaderTest) {
    typename TypeParam::MQType* mQueue2 = this->newQueue();

    ASSERT_NE(nullptr, mQueue2);

    const size_t dataLen = 16;
    ASSERT_LE(dataLen, this->mNumMessagesMax);

    bool ret = this->requestWriteFmqUnsync(dataLen, this->mService);
    ASSERT_TRUE(ret);

    pid_t pid;
    if ((pid = fork()) == 0) {
        /* child process */
        int32_t readData[dataLen] = {};
        ASSERT_TRUE(mQueue2->read(readData, dataLen));
        ASSERT_TRUE(verifyData(readData, dataLen));
        exit(0);
    } else {
        ASSERT_GT(pid,
                  0 /* parent should see PID greater than 0 for a good fork */);
        int32_t readData[dataLen] = {};
        ASSERT_TRUE(this->mQueue->read(readData, dataLen));
        ASSERT_TRUE(verifyData(readData, dataLen));
    }
}

/*
 * Request this->mService to write into the FMQ until it is full.
 * Request this->mService to do another write and verify it is successful.
 * Use two reader processes to read and verify that both fail.
 */
TYPED_TEST(UnsynchronizedWriteClient, OverflowNotificationTest) {
    typename TypeParam::MQType* mQueue2 = this->newQueue();
    ASSERT_NE(nullptr, mQueue2);

    bool ret = this->requestWriteFmqUnsync(this->mNumMessagesMax, this->mService);
    ASSERT_TRUE(ret);
    ret = this->requestWriteFmqUnsync(1, this->mService);
    ASSERT_TRUE(ret);

    pid_t pid;
    if ((pid = fork()) == 0) {
        /* child process */
        std::vector<int32_t> readData(this->mNumMessagesMax);
        ASSERT_FALSE(mQueue2->read(&readData[0], this->mNumMessagesMax));
        exit(0);
    } else {
        ASSERT_GT(pid, 0/* parent should see PID greater than 0 for a good fork */);
        std::vector<int32_t> readData(this->mNumMessagesMax);
        ASSERT_FALSE(this->mQueue->read(&readData[0], this->mNumMessagesMax));
    }
}

/*
 * Make sure a valid queue can be created with different supported types.
 * All fundamental or native types should work. An AIDL parcelable that is
 * annotated with @FixedSize is supported. A parcelable without it, will cause
 * a compilation error.
 */
typedef ::testing::Types<FixedParcelable, EventFlagBits, bool, int8_t, char, char16_t, int32_t,
                         int64_t, float, double>
        AidlTypeCheckTypes;

template <typename T>
class AidlTypeChecks : public ::testing::Test {};

TYPED_TEST_CASE(AidlTypeChecks, AidlTypeCheckTypes);

TYPED_TEST(AidlTypeChecks, FixedSizeParcelableTest) {
    android::AidlMessageQueue<TypeParam, UnsynchronizedWrite> queue =
            android::AidlMessageQueue<TypeParam, UnsynchronizedWrite>(64);
    ASSERT_TRUE(queue.isValid());
    // Make sure we can do a simple write/read of any value.
    TypeParam writeData[1];
    TypeParam readData[1];
    EXPECT_TRUE(queue.write(writeData, 1));
    EXPECT_TRUE(queue.read(readData, 1));
}
