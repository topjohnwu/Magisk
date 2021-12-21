/*
 * Copyright 2020 The Android Open Source Project
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

package android.fmq.test;

import android.hardware.common.fmq.MQDescriptor;
import android.hardware.common.fmq.SynchronizedReadWrite;
import android.hardware.common.fmq.UnsynchronizedWrite;

/**
 * Test interface for MQDescriptor.
 */
interface ITestAidlMsgQ {
    /**
     * This method requests the service to set up a synchronous read/write
     * wait-free FMQ using the input descriptor with the client as reader.
     *
     * @param mqDesc This structure describes the FMQ that was set up by the
     * client. Server uses this descriptor to set up a FMQ object at its end.
     *
     * @return True if the setup is successful.
     */
    boolean configureFmqSyncReadWrite(in MQDescriptor<int, SynchronizedReadWrite> mqDesc);

    /**
     * This method requests the service to return an MQDescriptor to
     * an unsynchronized FMQ set up by the server. If 'configureFmq' is
     * true, then the server sets up a new unsynchronized FMQ. This
     * method is to be used to test multiple reader processes.
     *
     * @param configureFmq The server sets up a new unsynchronized FMQ if
     * this parameter is true.
     * @param userFd True to initialize the message queue with a user supplied
     * file descriptor for the ring buffer.
     * False to let the message queue use a single FD for everything.
     *
     * @param out ret True if successful.
     * @param out mqDesc This structure describes the unsynchronized FMQ that was
     * set up by the service. Client can use it to set up the FMQ at its end.
     */
    boolean getFmqUnsyncWrite(in boolean configureFmq, in boolean userFd,
        out MQDescriptor<int, UnsynchronizedWrite> mqDesc);

    /**
     * This method requests the service to trigger a blocking read.
     *
     * @param count Number of messages to read.
     *
     */
    oneway void requestBlockingRead(in int count);

    /**
     * This method requests the service to trigger a blocking read using
     * default Event Flag notification bits defined by the MessageQueue class.
     *
     * @param count Number of messages to read.
     *
     */
    oneway void requestBlockingReadDefaultEventFlagBits(in int count);

    /**
     * This method requests the service to repeatedly trigger blocking reads.
     *
     * @param count Number of messages to read in a single blocking read.
     * @param numIter Number of blocking reads to trigger.
     *
     */
    oneway void requestBlockingReadRepeat(in int count, in int numIter);

    /**
     * This method request the service to read from the synchronized read/write
     * FMQ.
     *
     * @param count Number to messages to read.
     *
     * @return True if the read operation was successful.
     */
    boolean requestReadFmqSync(in int count);

    /**
     * This method request the service to read from the unsynchronized flavor of
     * FMQ.
     *
     * @param count Number to messages to read.
     *
     * @return Will be True if the read operation was successful.
     */
    boolean requestReadFmqUnsync(in int count);

    /**
     * This method request the service to write into the synchronized read/write
     * flavor of the FMQ.
     *
     * @param count Number to messages to write.
     *
     * @return True if the write operation was successful.
     */
    boolean requestWriteFmqSync(in int count);

    /**
     * This method request the service to write into the unsynchronized flavor
     * of FMQ.
     *
     * @param count Number to messages to write.
     *
     * @return True if the write operation was successful.
     */
    boolean requestWriteFmqUnsync(in int count);
}
