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

#include <android-base/logging.h>

#include <gtest/gtest.h>
#include <utils/StrongPointer.h>
#include <chrono>
#include <iostream>

#include <android/hardware/tests/msgq/1.0/IBenchmarkMsgQ.h>
#include <fmq/MessageQueue.h>
#include <hidl/ServiceManagement.h>

// libutils:
using android::OK;
using android::sp;
using android::status_t;

// generated
using android::hardware::tests::msgq::V1_0::IBenchmarkMsgQ;
using std::cerr;
using std::cout;
using std::endl;

// libhidl
using android::hardware::kSynchronizedReadWrite;
using android::hardware::MQDescriptorSync;
using android::hardware::MessageQueue;
using android::hardware::details::waitForHwService;

/*
 * All the benchmark cases will be performed on an FMQ of size kQueueSize.
 */
static const int32_t kQueueSize = 1024 * 16;

/*
 * The number of iterations for each experiment.
 */
static const uint32_t kNumIterations = 1000;

/*
 * The various packet sizes used are as follows.
 */
enum PacketSizes {
    kPacketSize64 = 64,
    kPacketSize128 = 128,
    kPacketSize256 = 256,
    kPacketSize512 = 512,
    kPacketSize1024 = 1024
};

class MQTestClient : public ::testing::Test {
protected:
    virtual void TearDown() {
        delete mFmqInbox;
        delete mFmqOutbox;
    }

    virtual void SetUp() {
        // waitForHwService is required because IBenchmarkMsgQ is not in manifest.xml.
        // "Real" HALs shouldn't be doing this.
        waitForHwService(IBenchmarkMsgQ::descriptor, "default");
        service = IBenchmarkMsgQ::getService();
        ASSERT_NE(service, nullptr);
        ASSERT_TRUE(service->isRemote());
        /*
         * Request service to configure the client inbox queue.
         */
        service->configureClientInboxSyncReadWrite([this](bool ret,
                                                          const MQDescriptorSync<uint8_t>& in) {
          ASSERT_TRUE(ret);
          mFmqInbox = new (std::nothrow) MessageQueue<uint8_t, kSynchronizedReadWrite>(in);
        });

        ASSERT_TRUE(mFmqInbox != nullptr);
        ASSERT_TRUE(mFmqInbox->isValid());
        /*
         * Reqeust service to configure the client outbox queue.
         */
        service->configureClientOutboxSyncReadWrite([this](bool ret,
                                                           const MQDescriptorSync<uint8_t>& out) {
         ASSERT_TRUE(ret);
          mFmqOutbox = new (std::nothrow) MessageQueue<uint8_t,
                             kSynchronizedReadWrite>(out);
        });

        ASSERT_TRUE(mFmqOutbox != nullptr);
        ASSERT_TRUE(mFmqOutbox->isValid());
    }

    sp<IBenchmarkMsgQ> service;
    android::hardware::MessageQueue<uint8_t, kSynchronizedReadWrite>* mFmqInbox = nullptr;
    android::hardware::MessageQueue<uint8_t, kSynchronizedReadWrite>* mFmqOutbox = nullptr;
};

/*
 * Client writes a 64 byte packet into the outbox queue, service reads the
 * same and
 * writes the packet into the client's inbox queue. Client reads the packet. The
 * average time taken for the cycle is measured.
 */
TEST_F(MQTestClient, BenchMarkMeasurePingPongTransfer) {
    uint8_t* data = new (std::nothrow) uint8_t[kPacketSize64];
    ASSERT_TRUE(data != nullptr);
    int64_t accumulatedTime = 0;
    size_t numRoundTrips = 0;

    /*
     * This method requests the service to create a thread which reads
     * from mFmqOutbox and writes into mFmqInbox.
     */
    service->benchmarkPingPong(kNumIterations);
    std::chrono::time_point<std::chrono::high_resolution_clock> timeStart =
            std::chrono::high_resolution_clock::now();
    while (numRoundTrips < kNumIterations) {
        while (mFmqOutbox->write(data, kPacketSize64) == 0) {
        }

        while (mFmqInbox->read(data, kPacketSize64) == 0) {
        }

        numRoundTrips++;
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd =
            std::chrono::high_resolution_clock::now();
    accumulatedTime += static_cast<int64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
            timeEnd - timeStart).count());
    accumulatedTime /= kNumIterations;

    cout << "Round trip time for " << kPacketSize64 << "bytes: " <<
         accumulatedTime << "ns" << endl;
    delete[] data;
}

/*
 * Measure the average time taken to read 64 bytes from the queue.
 */
TEST_F(MQTestClient, BenchMarkMeasureRead64Bytes) {
    uint8_t* data = new (std::nothrow) uint8_t[kPacketSize64];
    ASSERT_TRUE(data != nullptr);

    uint32_t numLoops = kQueueSize / kPacketSize64;
    uint64_t accumulatedTime = 0;
    for (uint32_t i = 0; i < kNumIterations; i++) {
        bool ret = service->requestWrite(kQueueSize);
        ASSERT_TRUE(ret);
        std::chrono::time_point<std::chrono::high_resolution_clock> timeStart =
                std::chrono::high_resolution_clock::now();
        /*
         * The read() method returns true only if the the correct number of bytes
         * were succesfully read from the queue.
         */
        for (uint32_t j = 0; j < numLoops; j++) {
            ASSERT_TRUE(mFmqInbox->read(data, kPacketSize64));
        }

        std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd =
                std::chrono::high_resolution_clock::now();
        accumulatedTime += (timeEnd - timeStart).count();
    }

    accumulatedTime /= (numLoops * kNumIterations);
    cout << "Average time to read" << kPacketSize64
         << "bytes: " << accumulatedTime << "ns" << endl;
    delete[] data;
}

/*
 * Measure the average time taken to read 128 bytes.
 */
TEST_F(MQTestClient, BenchMarkMeasureRead128Bytes) {
    uint8_t* data = new (std::nothrow) uint8_t[kPacketSize128];
    ASSERT_TRUE(data != nullptr);

    uint32_t numLoops = kQueueSize / kPacketSize128;
    uint64_t accumulatedTime = 0;

    for (uint32_t i = 0; i < kNumIterations; i++) {
        bool ret = service->requestWrite(kQueueSize);
        ASSERT_TRUE(ret);
        std::chrono::time_point<std::chrono::high_resolution_clock> timeStart =
                std::chrono::high_resolution_clock::now();

        /*
         * The read() method returns true only if the the correct number of bytes
         * were succesfully read from the queue.
         */
        for (uint32_t j = 0; j < numLoops; j++) {
            ASSERT_TRUE(mFmqInbox->read(data, kPacketSize128));
        }
        std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd =
                std::chrono::high_resolution_clock::now();
        accumulatedTime += (timeEnd - timeStart).count();
    }

    accumulatedTime /= (numLoops * kNumIterations);
    cout << "Average time to read" << kPacketSize128
         << "bytes: " << accumulatedTime << "ns" << endl;
    delete[] data;
}

/*
 * Measure the average time taken to read 256 bytes from the queue.
 */
TEST_F(MQTestClient, BenchMarkMeasureRead256Bytes) {
    uint8_t* data = new (std::nothrow) uint8_t[kPacketSize256];
    ASSERT_TRUE(data != nullptr);
    uint32_t numLoops = kQueueSize / kPacketSize256;
    uint64_t accumulatedTime = 0;

    for (uint32_t i = 0; i < kNumIterations; i++) {
        bool ret = service->requestWrite(kQueueSize);
        ASSERT_TRUE(ret);
        std::chrono::time_point<std::chrono::high_resolution_clock> timeStart =
                std::chrono::high_resolution_clock::now();
        /*
         * The read() method returns true only if the the correct number of bytes
         * were succesfully read from the queue.
         */
        for (uint32_t j = 0; j < numLoops; j++) {
            ASSERT_TRUE(mFmqInbox->read(data, kPacketSize256));
        }

        std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd =
                std::chrono::high_resolution_clock::now();
        accumulatedTime += (timeEnd - timeStart).count();
    }

    accumulatedTime /= (numLoops * kNumIterations);
    cout << "Average time to read" << kPacketSize256
         << "bytes: " << accumulatedTime << "ns" << endl;
    delete[] data;
}

/*
 * Measure the average time taken to read 512 bytes from the queue.
 */
TEST_F(MQTestClient, BenchMarkMeasureRead512Bytes) {
    uint8_t* data = new (std::nothrow) uint8_t[kPacketSize512];
    ASSERT_TRUE(data != nullptr);
    uint32_t numLoops = kQueueSize / kPacketSize512;
    uint64_t accumulatedTime = 0;
    for (uint32_t i = 0; i < kNumIterations; i++) {
        bool ret = service->requestWrite(kQueueSize);
        ASSERT_TRUE(ret);
        std::chrono::time_point<std::chrono::high_resolution_clock> timeStart =
                std::chrono::high_resolution_clock::now();
        /*
         * The read() method returns true only if the the correct number of bytes
         * were succesfully read from the queue.
         */
        for (uint32_t j = 0; j < numLoops; j++) {
            ASSERT_TRUE(mFmqInbox->read(data, kPacketSize512));
        }
        std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd =
                std::chrono::high_resolution_clock::now();
        accumulatedTime += (timeEnd - timeStart).count();
    }

    accumulatedTime /= (numLoops * kNumIterations);
    cout << "Average time to read" << kPacketSize512
         << "bytes: " << accumulatedTime << "ns" << endl;
    delete[] data;
}

/*
 * Measure the average time taken to write 64 bytes into the queue.
 */
TEST_F(MQTestClient, BenchMarkMeasureWrite64Bytes) {
    uint8_t* data = new (std::nothrow) uint8_t[kPacketSize64];
    ASSERT_TRUE(data != nullptr);
    uint32_t numLoops = kQueueSize / kPacketSize64;
    uint64_t accumulatedTime = 0;

    for (uint32_t i = 0; i < kNumIterations; i++) {
        std::chrono::time_point<std::chrono::high_resolution_clock> timeStart =
                std::chrono::high_resolution_clock::now();
        /*
         * Write until the queue is full and request service to empty the queue.
         */
        for (uint32_t j = 0; j < numLoops; j++) {
            bool result = mFmqOutbox->write(data, kPacketSize64);
            ASSERT_TRUE(result);
        }

        std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd =
                std::chrono::high_resolution_clock::now();
        accumulatedTime += (timeEnd - timeStart).count();

        bool ret = service->requestRead(kQueueSize);
        ASSERT_TRUE(ret);
    }

    accumulatedTime /= (numLoops * kNumIterations);
    cout << "Average time to write " << kPacketSize64
         << "bytes: " << accumulatedTime << "ns" << endl;
    delete[] data;
}

/*
 * Measure the average time taken to write 128 bytes into the queue.
 */
TEST_F(MQTestClient, BenchMarkMeasureWrite128Bytes) {
    uint8_t* data = new (std::nothrow) uint8_t[kPacketSize128];
    ASSERT_TRUE(data != nullptr);
    uint32_t numLoops = kQueueSize / kPacketSize128;
    uint64_t accumulatedTime = 0;

    for (uint32_t i = 0; i < kNumIterations; i++) {
        std::chrono::time_point<std::chrono::high_resolution_clock> timeStart =
                std::chrono::high_resolution_clock::now();
        /*
         * Write until the queue is full and request service to empty the queue.
         */
        for (uint32_t j = 0; j < numLoops; j++) {
            ASSERT_TRUE(mFmqOutbox->write(data, kPacketSize128));
        }

        std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd =
                std::chrono::high_resolution_clock::now();
        accumulatedTime += (timeEnd - timeStart).count();

        bool ret = service->requestRead(kQueueSize);
        ASSERT_TRUE(ret);
    }

    accumulatedTime /= (numLoops * kNumIterations);
    cout << "Average time to write " << kPacketSize128
         << "bytes: " << accumulatedTime << "ns" << endl;
    delete[] data;
}

/*
 * Measure the average time taken to write 256 bytes into the queue.
 */
TEST_F(MQTestClient, BenchMarkMeasureWrite256Bytes) {
    uint8_t* data = new (std::nothrow) uint8_t[kPacketSize256];
    ASSERT_TRUE(data != nullptr);
    uint32_t numLoops = kQueueSize / kPacketSize256;
    uint64_t accumulatedTime = 0;

    for (uint32_t i = 0; i < kNumIterations; i++) {
        std::chrono::time_point<std::chrono::high_resolution_clock> timeStart =
                std::chrono::high_resolution_clock::now();
        /*
         * Write until the queue is full and request service to empty the queue.
         */
        for (uint32_t j = 0; j < numLoops; j++) {
            ASSERT_TRUE(mFmqOutbox->write(data, kPacketSize256));
        }
        std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd =
                std::chrono::high_resolution_clock::now();
        accumulatedTime += (timeEnd - timeStart).count();

        bool ret = service->requestRead(kQueueSize);
        ASSERT_TRUE(ret);
    }

    accumulatedTime /= (numLoops * kNumIterations);
    cout << "Average time to write " << kPacketSize256
         << "bytes: " << accumulatedTime << "ns" << endl;
    delete[] data;
}

/*
 * Measure the average time taken to write 512 bytes into the queue.
 */
TEST_F(MQTestClient, BenchMarkMeasureWrite512Bytes) {
    uint8_t* data = new (std::nothrow) uint8_t[kPacketSize512];
    ASSERT_TRUE(data != nullptr);
    uint32_t numLoops = kQueueSize / kPacketSize512;
    uint64_t accumulatedTime = 0;

    for (uint32_t i = 0; i < kNumIterations; i++) {
        std::chrono::time_point<std::chrono::high_resolution_clock> timeStart =
                std::chrono::high_resolution_clock::now();

        /*
         * Write until the queue is full and request service to empty the queue.
         * The write() method returns true only if the specified number of bytes
         * were succesfully written.
         */
        for (uint32_t j = 0; j < numLoops; j++) {
            ASSERT_TRUE(mFmqOutbox->write(data, kPacketSize512));
        }

        std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd =
                std::chrono::high_resolution_clock::now();
        accumulatedTime += (timeEnd - timeStart).count();

        bool ret = service->requestRead(kQueueSize);
        ASSERT_TRUE(ret);
    }

    accumulatedTime /= (numLoops * kNumIterations);
    cout << "Average time to write " << kPacketSize512
         << "bytes: " << accumulatedTime << "ns" << endl;
    delete[] data;
}

/*
 * Service continuously writes a packet of 64 bytes into the client's inbox
 * queue
 * of size 16K. Client keeps reading from the inbox queue. The average write to
 * read delay is calculated.
 */
TEST_F(MQTestClient, BenchMarkMeasureServiceWriteClientRead) {
    uint8_t* data = new (std::nothrow) uint8_t[kPacketSize64];
    ASSERT_TRUE(data != nullptr);
    /*
     * This method causes the service to create a thread which writes
     * into the mFmqInbox queue kNumIterations packets.
     */
    service->benchmarkServiceWriteClientRead(kNumIterations);
    android::hardware::hidl_vec<int64_t> clientRcvTimeArray;
    clientRcvTimeArray.resize(kNumIterations);
    for (uint32_t i = 0; i < kNumIterations; i++) {
        do {
            clientRcvTimeArray[i] =
                    std::chrono::high_resolution_clock::now().time_since_epoch().count();
        } while (mFmqInbox->read(data, kPacketSize64) == 0);
    }
    service->sendTimeData(clientRcvTimeArray);
    delete[] data;
}
