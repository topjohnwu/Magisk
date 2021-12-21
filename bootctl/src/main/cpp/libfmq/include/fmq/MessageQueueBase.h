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

#pragma once

#include <android-base/unique_fd.h>
#include <cutils/ashmem.h>
#include <fmq/EventFlag.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>
#include <atomic>
#include <new>

using android::hardware::kSynchronizedReadWrite;
using android::hardware::kUnsynchronizedWrite;
using android::hardware::MQFlavor;

namespace android {

template <template <typename, MQFlavor> class MQDescriptorType, typename T, MQFlavor flavor>
struct MessageQueueBase {
    typedef MQDescriptorType<T, flavor> Descriptor;

    /**
     * @param Desc MQDescriptor describing the FMQ.
     * @param resetPointers bool indicating whether the read/write pointers
     * should be reset or not.
     */
    MessageQueueBase(const Descriptor& Desc, bool resetPointers = true);

    ~MessageQueueBase();

    /**
     * This constructor uses Ashmem shared memory to create an FMQ
     * that can contain a maximum of 'numElementsInQueue' elements of type T.
     *
     * @param numElementsInQueue Capacity of the MessageQueue in terms of T.
     * @param configureEventFlagWord Boolean that specifies if memory should
     * also be allocated and mapped for an EventFlag word.
     * @param bufferFd User-supplied file descriptor to map the memory for the ringbuffer
     * By default, bufferFd=-1 means library will allocate ashmem region for ringbuffer.
     * MessageQueue takes ownership of the file descriptor.
     * @param bufferSize size of buffer in bytes that bufferFd represents. This
     * size must be larger than or equal to (numElementsInQueue * sizeof(T)).
     * Otherwise, operations will cause out-of-bounds memory access.
     */

    MessageQueueBase(size_t numElementsInQueue, bool configureEventFlagWord,
                     android::base::unique_fd bufferFd, size_t bufferSize);

    MessageQueueBase(size_t numElementsInQueue, bool configureEventFlagWord = false)
        : MessageQueueBase(numElementsInQueue, configureEventFlagWord, android::base::unique_fd(),
                           0) {}

    /**
     * @return Number of items of type T that can be written into the FMQ
     * without a read.
     */
    size_t availableToWrite() const;

    /**
     * @return Number of items of type T that are waiting to be read from the
     * FMQ.
     */
    size_t availableToRead() const;

    /**
     * Returns the size of type T in bytes.
     *
     * @param Size of T.
     */
    size_t getQuantumSize() const;

    /**
     * Returns the size of the FMQ in terms of the size of type T.
     *
     * @return Number of items of type T that will fit in the FMQ.
     */
    size_t getQuantumCount() const;

    /**
     * @return Whether the FMQ is configured correctly.
     */
    bool isValid() const;

    /**
     * Non-blocking write to FMQ.
     *
     * @param data Pointer to the object of type T to be written into the FMQ.
     *
     * @return Whether the write was successful.
     */
    bool write(const T* data);

    /**
     * Non-blocking read from FMQ.
     *
     * @param data Pointer to the memory where the object read from the FMQ is
     * copied to.
     *
     * @return Whether the read was successful.
     */
    bool read(T* data);

    /**
     * Write some data into the FMQ without blocking.
     *
     * @param data Pointer to the array of items of type T.
     * @param count Number of items in array.
     *
     * @return Whether the write was successful.
     */
    bool write(const T* data, size_t count);

    /**
     * Perform a blocking write of 'count' items into the FMQ using EventFlags.
     * Does not support partial writes.
     *
     * If 'evFlag' is nullptr, it is checked whether there is an EventFlag object
     * associated with the FMQ and it is used in that case.
     *
     * The application code must ensure that 'evFlag' used by the
     * reader(s)/writer is based upon the same EventFlag word.
     *
     * The method will return false without blocking if any of the following
     * conditions are true:
     * - If 'evFlag' is nullptr and the FMQ does not own an EventFlag object.
     * - If the 'readNotification' bit mask is zero.
     * - If 'count' is greater than the FMQ size.
     *
     * If the there is insufficient space available to write into it, the
     * EventFlag bit mask 'readNotification' is is waited upon.
     *
     * This method should only be used with a MessageQueue of the flavor
     * 'kSynchronizedReadWrite'.
     *
     * Upon a successful write, wake is called on 'writeNotification' (if
     * non-zero).
     *
     * @param data Pointer to the array of items of type T.
     * @param count Number of items in array.
     * @param readNotification The EventFlag bit mask to wait on if there is not
     * enough space in FMQ to write 'count' items.
     * @param writeNotification The EventFlag bit mask to call wake on
     * a successful write. No wake is called if 'writeNotification' is zero.
     * @param timeOutNanos Number of nanoseconds after which the blocking
     * write attempt is aborted.
     * @param evFlag The EventFlag object to be used for blocking. If nullptr,
     * it is checked whether the FMQ owns an EventFlag object and that is used
     * for blocking instead.
     *
     * @return Whether the write was successful.
     */
    bool writeBlocking(const T* data, size_t count, uint32_t readNotification,
                       uint32_t writeNotification, int64_t timeOutNanos = 0,
                       android::hardware::EventFlag* evFlag = nullptr);

    bool writeBlocking(const T* data, size_t count, int64_t timeOutNanos = 0);

    /**
     * Read some data from the FMQ without blocking.
     *
     * @param data Pointer to the array to which read data is to be written.
     * @param count Number of items to be read.
     *
     * @return Whether the read was successful.
     */
    bool read(T* data, size_t count);

    /**
     * Perform a blocking read operation of 'count' items from the FMQ. Does not
     * perform a partial read.
     *
     * If 'evFlag' is nullptr, it is checked whether there is an EventFlag object
     * associated with the FMQ and it is used in that case.
     *
     * The application code must ensure that 'evFlag' used by the
     * reader(s)/writer is based upon the same EventFlag word.
     *
     * The method will return false without blocking if any of the following
     * conditions are true:
     * -If 'evFlag' is nullptr and the FMQ does not own an EventFlag object.
     * -If the 'writeNotification' bit mask is zero.
     * -If 'count' is greater than the FMQ size.
     *
     * This method should only be used with a MessageQueue of the flavor
     * 'kSynchronizedReadWrite'.

     * If FMQ does not contain 'count' items, the eventFlag bit mask
     * 'writeNotification' is waited upon. Upon a successful read from the FMQ,
     * wake is called on 'readNotification' (if non-zero).
     *
     * @param data Pointer to the array to which read data is to be written.
     * @param count Number of items to be read.
     * @param readNotification The EventFlag bit mask to call wake on after
     * a successful read. No wake is called if 'readNotification' is zero.
     * @param writeNotification The EventFlag bit mask to call a wait on
     * if there is insufficient data in the FMQ to be read.
     * @param timeOutNanos Number of nanoseconds after which the blocking
     * read attempt is aborted.
     * @param evFlag The EventFlag object to be used for blocking.
     *
     * @return Whether the read was successful.
     */
    bool readBlocking(T* data, size_t count, uint32_t readNotification, uint32_t writeNotification,
                      int64_t timeOutNanos = 0, android::hardware::EventFlag* evFlag = nullptr);

    bool readBlocking(T* data, size_t count, int64_t timeOutNanos = 0);

    /**
     * Get a pointer to the MQDescriptor object that describes this FMQ.
     *
     * @return Pointer to the MQDescriptor associated with the FMQ.
     */
    const Descriptor* getDesc() const { return mDesc.get(); }

    /**
     * Get a pointer to the EventFlag word if there is one associated with this FMQ.
     *
     * @return Pointer to an EventFlag word, will return nullptr if not
     * configured. This method does not transfer ownership. The EventFlag
     * word will be unmapped by the MessageQueue destructor.
     */
    std::atomic<uint32_t>* getEventFlagWord() const { return mEvFlagWord; }

    /**
     * Describes a memory region in the FMQ.
     */
    struct MemRegion {
        MemRegion() : MemRegion(nullptr, 0) {}

        MemRegion(T* base, size_t size) : address(base), length(size) {}

        MemRegion& operator=(const MemRegion& other) {
            address = other.address;
            length = other.length;
            return *this;
        }

        /**
         * Gets a pointer to the base address of the MemRegion.
         */
        inline T* getAddress() const { return address; }

        /**
         * Gets the length of the MemRegion. This would equal to the number
         * of items of type T that can be read from/written into the MemRegion.
         */
        inline size_t getLength() const { return length; }

        /**
         * Gets the length of the MemRegion in bytes.
         */
        inline size_t getLengthInBytes() const { return length * sizeof(T); }

      private:
        /* Base address */
        T* address;

        /*
         * Number of items of type T that can be written to/read from the base
         * address.
         */
        size_t length;
    };

    /**
     * Describes the memory regions to be used for a read or write.
     * The struct contains two MemRegion objects since the FMQ is a ring
     * buffer and a read or write operation can wrap around. A single message
     * of type T will never be broken between the two MemRegions.
     */
    struct MemTransaction {
        MemTransaction() : MemTransaction(MemRegion(), MemRegion()) {}

        MemTransaction(const MemRegion& regionFirst, const MemRegion& regionSecond)
            : first(regionFirst), second(regionSecond) {}

        MemTransaction& operator=(const MemTransaction& other) {
            first = other.first;
            second = other.second;
            return *this;
        }

        /**
         * Helper method to calculate the address for a particular index for
         * the MemTransaction object.
         *
         * @param idx Index of the slot to be read/written. If the
         * MemTransaction object is representing the memory region to read/write
         * N items of type T, the valid range of idx is between 0 and N-1.
         *
         * @return Pointer to the slot idx. Will be nullptr for an invalid idx.
         */
        T* getSlot(size_t idx);

        /**
         * Helper method to write 'nMessages' items of type T into the memory
         * regions described by the object starting from 'startIdx'. This method
         * uses memcpy() and is not to meant to be used for a zero copy operation.
         * Partial writes are not supported.
         *
         * @param data Pointer to the source buffer.
         * @param nMessages Number of items of type T.
         * @param startIdx The slot number to begin the write from. If the
         * MemTransaction object is representing the memory region to read/write
         * N items of type T, the valid range of startIdx is between 0 and N-1;
         *
         * @return Whether the write operation of size 'nMessages' succeeded.
         */
        bool copyTo(const T* data, size_t startIdx, size_t nMessages = 1);

        /*
         * Helper method to read 'nMessages' items of type T from the memory
         * regions described by the object starting from 'startIdx'. This method uses
         * memcpy() and is not meant to be used for a zero copy operation. Partial reads
         * are not supported.
         *
         * @param data Pointer to the destination buffer.
         * @param nMessages Number of items of type T.
         * @param startIdx The slot number to begin the read from. If the
         * MemTransaction object is representing the memory region to read/write
         * N items of type T, the valid range of startIdx is between 0 and N-1.
         *
         * @return Whether the read operation of size 'nMessages' succeeded.
         */
        bool copyFrom(T* data, size_t startIdx, size_t nMessages = 1);

        /**
         * Returns a const reference to the first MemRegion in the
         * MemTransaction object.
         */
        inline const MemRegion& getFirstRegion() const { return first; }

        /**
         * Returns a const reference to the second MemRegion in the
         * MemTransaction object.
         */
        inline const MemRegion& getSecondRegion() const { return second; }

      private:
        /*
         * Given a start index and the number of messages to be
         * read/written, this helper method calculates the
         * number of messages that should should be written to both the first
         * and second MemRegions and the base addresses to be used for
         * the read/write operation.
         *
         * Returns false if the 'startIdx' and 'nMessages' is
         * invalid for the MemTransaction object.
         */
        bool inline getMemRegionInfo(size_t idx, size_t nMessages, size_t& firstCount,
                                     size_t& secondCount, T** firstBaseAddress,
                                     T** secondBaseAddress);
        MemRegion first;
        MemRegion second;
    };

    /**
     * Get a MemTransaction object to write 'nMessages' items of type T.
     * Once the write is performed using the information from MemTransaction,
     * the write operation is to be committed using a call to commitWrite().
     *
     * @param nMessages Number of messages of type T.
     * @param Pointer to MemTransaction struct that describes memory to write 'nMessages'
     * items of type T. If a write of size 'nMessages' is not possible, the base
     * addresses in the MemTransaction object would be set to nullptr.
     *
     * @return Whether it is possible to write 'nMessages' items of type T
     * into the FMQ.
     */
    bool beginWrite(size_t nMessages, MemTransaction* memTx) const;

    /**
     * Commit a write of size 'nMessages'. To be only used after a call to beginWrite().
     *
     * @param nMessages number of messages of type T to be written.
     *
     * @return Whether the write operation of size 'nMessages' succeeded.
     */
    bool commitWrite(size_t nMessages);

    /**
     * Get a MemTransaction object to read 'nMessages' items of type T.
     * Once the read is performed using the information from MemTransaction,
     * the read operation is to be committed using a call to commitRead().
     *
     * @param nMessages Number of messages of type T.
     * @param pointer to MemTransaction struct that describes memory to read 'nMessages'
     * items of type T. If a read of size 'nMessages' is not possible, the base
     * pointers in the MemTransaction object returned will be set to nullptr.
     *
     * @return bool Whether it is possible to read 'nMessages' items of type T
     * from the FMQ.
     */
    bool beginRead(size_t nMessages, MemTransaction* memTx) const;

    /**
     * Commit a read of size 'nMessages'. To be only used after a call to beginRead().
     * For the unsynchronized flavor of FMQ, this method will return a failure
     * if a write overflow happened after beginRead() was invoked.
     *
     * @param nMessages number of messages of type T to be read.
     *
     * @return bool Whether the read operation of size 'nMessages' succeeded.
     */
    bool commitRead(size_t nMessages);

  private:
    size_t availableToWriteBytes() const;
    size_t availableToReadBytes() const;

    MessageQueueBase(const MessageQueueBase& other) = delete;
    MessageQueueBase& operator=(const MessageQueueBase& other) = delete;

    void* mapGrantorDescr(uint32_t grantorIdx);
    void unmapGrantorDescr(void* address, uint32_t grantorIdx);
    void initMemory(bool resetPointers);

    enum DefaultEventNotification : uint32_t {
        /*
         * These are only used internally by the readBlocking()/writeBlocking()
         * methods and hence once other bit combinations are not required.
         */
        FMQ_NOT_FULL = 0x01,
        FMQ_NOT_EMPTY = 0x02
    };
    std::unique_ptr<Descriptor> mDesc;
    uint8_t* mRing = nullptr;
    /*
     * TODO(b/31550092): Change to 32 bit read and write pointer counters.
     */
    std::atomic<uint64_t>* mReadPtr = nullptr;
    std::atomic<uint64_t>* mWritePtr = nullptr;

    std::atomic<uint32_t>* mEvFlagWord = nullptr;

    /*
     * This EventFlag object will be owned by the FMQ and will have the same
     * lifetime.
     */
    android::hardware::EventFlag* mEventFlag = nullptr;
};

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
T* MessageQueueBase<MQDescriptorType, T, flavor>::MemTransaction::getSlot(size_t idx) {
    size_t firstRegionLength = first.getLength();
    size_t secondRegionLength = second.getLength();

    if (idx > firstRegionLength + secondRegionLength) {
        return nullptr;
    }

    if (idx < firstRegionLength) {
        return first.getAddress() + idx;
    }

    return second.getAddress() + idx - firstRegionLength;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::MemTransaction::getMemRegionInfo(
        size_t startIdx, size_t nMessages, size_t& firstCount, size_t& secondCount,
        T** firstBaseAddress, T** secondBaseAddress) {
    size_t firstRegionLength = first.getLength();
    size_t secondRegionLength = second.getLength();

    if (startIdx + nMessages > firstRegionLength + secondRegionLength) {
        /*
         * Return false if 'nMessages' starting at 'startIdx' cannot be
         * accommodated by the MemTransaction object.
         */
        return false;
    }

    /* Number of messages to be read/written to the first MemRegion. */
    firstCount =
            startIdx < firstRegionLength ? std::min(nMessages, firstRegionLength - startIdx) : 0;

    /* Number of messages to be read/written to the second MemRegion. */
    secondCount = nMessages - firstCount;

    if (firstCount != 0) {
        *firstBaseAddress = first.getAddress() + startIdx;
    }

    if (secondCount != 0) {
        size_t secondStartIdx = startIdx > firstRegionLength ? startIdx - firstRegionLength : 0;
        *secondBaseAddress = second.getAddress() + secondStartIdx;
    }

    return true;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::MemTransaction::copyFrom(T* data,
                                                                             size_t startIdx,
                                                                             size_t nMessages) {
    if (data == nullptr) {
        return false;
    }

    size_t firstReadCount = 0, secondReadCount = 0;
    T *firstBaseAddress = nullptr, *secondBaseAddress = nullptr;

    if (getMemRegionInfo(startIdx, nMessages, firstReadCount, secondReadCount, &firstBaseAddress,
                         &secondBaseAddress) == false) {
        /*
         * Returns false if 'startIdx' and 'nMessages' are invalid for this
         * MemTransaction object.
         */
        return false;
    }

    if (firstReadCount != 0) {
        memcpy(data, firstBaseAddress, firstReadCount * sizeof(T));
    }

    if (secondReadCount != 0) {
        memcpy(data + firstReadCount, secondBaseAddress, secondReadCount * sizeof(T));
    }

    return true;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::MemTransaction::copyTo(const T* data,
                                                                           size_t startIdx,
                                                                           size_t nMessages) {
    if (data == nullptr) {
        return false;
    }

    size_t firstWriteCount = 0, secondWriteCount = 0;
    T *firstBaseAddress = nullptr, *secondBaseAddress = nullptr;

    if (getMemRegionInfo(startIdx, nMessages, firstWriteCount, secondWriteCount, &firstBaseAddress,
                         &secondBaseAddress) == false) {
        /*
         * Returns false if 'startIdx' and 'nMessages' are invalid for this
         * MemTransaction object.
         */
        return false;
    }

    if (firstWriteCount != 0) {
        memcpy(firstBaseAddress, data, firstWriteCount * sizeof(T));
    }

    if (secondWriteCount != 0) {
        memcpy(secondBaseAddress, data + firstWriteCount, secondWriteCount * sizeof(T));
    }

    return true;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
void MessageQueueBase<MQDescriptorType, T, flavor>::initMemory(bool resetPointers) {
    /*
     * Verify that the Descriptor contains the minimum number of grantors
     * the native_handle is valid and T matches quantum size.
     */
    if ((mDesc == nullptr) || !mDesc->isHandleValid() ||
        (mDesc->countGrantors() < hardware::details::kMinGrantorCount)) {
        return;
    }
    if (mDesc->getQuantum() != sizeof(T)) {
        hardware::details::logError(
                "Payload size differs between the queue instantiation and the "
                "MQDescriptor.");
        return;
    }

    const auto& grantors = mDesc->grantors();
    for (const auto& grantor : grantors) {
        if (hardware::details::isAlignedToWordBoundary(grantor.offset) == false) {
#ifdef __BIONIC__
            __assert(__FILE__, __LINE__, "Grantor offsets need to be aligned");
#endif
        }
    }

    if (flavor == kSynchronizedReadWrite) {
        mReadPtr = reinterpret_cast<std::atomic<uint64_t>*>(
                mapGrantorDescr(hardware::details::READPTRPOS));
    } else {
        /*
         * The unsynchronized write flavor of the FMQ may have multiple readers
         * and each reader would have their own read pointer counter.
         */
        mReadPtr = new (std::nothrow) std::atomic<uint64_t>;
    }
    if (mReadPtr == nullptr) {
#ifdef __BIONIC__
        __assert(__FILE__, __LINE__, "mReadPtr is null");
#endif
    }

    mWritePtr = reinterpret_cast<std::atomic<uint64_t>*>(
            mapGrantorDescr(hardware::details::WRITEPTRPOS));
    if (mWritePtr == nullptr) {
#ifdef __BIONIC__
        __assert(__FILE__, __LINE__, "mWritePtr is null");
#endif
    }

    if (resetPointers) {
        mReadPtr->store(0, std::memory_order_release);
        mWritePtr->store(0, std::memory_order_release);
    } else if (flavor != kSynchronizedReadWrite) {
        // Always reset the read pointer.
        mReadPtr->store(0, std::memory_order_release);
    }

    mRing = reinterpret_cast<uint8_t*>(mapGrantorDescr(hardware::details::DATAPTRPOS));
    if (mRing == nullptr) {
#ifdef __BIONIC__
        __assert(__FILE__, __LINE__, "mRing is null");
#endif
    }

    if (mDesc->countGrantors() > hardware::details::EVFLAGWORDPOS) {
        mEvFlagWord = static_cast<std::atomic<uint32_t>*>(
                mapGrantorDescr(hardware::details::EVFLAGWORDPOS));
        if (mEvFlagWord != nullptr) {
            android::hardware::EventFlag::createEventFlag(mEvFlagWord, &mEventFlag);
        } else {
#ifdef __BIONIC__
            __assert(__FILE__, __LINE__, "mEvFlagWord is null");
#endif
        }
    }
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
MessageQueueBase<MQDescriptorType, T, flavor>::MessageQueueBase(const Descriptor& Desc,
                                                                bool resetPointers) {
    mDesc = std::unique_ptr<Descriptor>(new (std::nothrow) Descriptor(Desc));
    if (mDesc == nullptr) {
        return;
    }

    initMemory(resetPointers);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
MessageQueueBase<MQDescriptorType, T, flavor>::MessageQueueBase(size_t numElementsInQueue,
                                                                bool configureEventFlagWord,
                                                                android::base::unique_fd bufferFd,
                                                                size_t bufferSize) {
    // Check if the buffer size would not overflow size_t
    if (numElementsInQueue > SIZE_MAX / sizeof(T)) {
        hardware::details::logError("Requested message queue size too large. Size of elements: " +
                                    std::to_string(sizeof(T)) +
                                    ". Number of elements: " + std::to_string(numElementsInQueue));
        return;
    }
    if (bufferFd != -1 && numElementsInQueue * sizeof(T) > bufferSize) {
        hardware::details::logError("The supplied buffer size(" + std::to_string(bufferSize) +
                                    ") is smaller than the required size(" +
                                    std::to_string(numElementsInQueue * sizeof(T)) + ").");
        return;
    }
    /*
     * The FMQ needs to allocate memory for the ringbuffer as well as for the
     * read and write pointer counters. If an EventFlag word is to be configured,
     * we also need to allocate memory for the same/
     */
    size_t kQueueSizeBytes = numElementsInQueue * sizeof(T);
    size_t kMetaDataSize = 2 * sizeof(android::hardware::details::RingBufferPosition);

    if (configureEventFlagWord) {
        kMetaDataSize += sizeof(std::atomic<uint32_t>);
    }

    /*
     * Ashmem memory region size needs to be specified in page-aligned bytes.
     * kQueueSizeBytes needs to be aligned to word boundary so that all offsets
     * in the grantorDescriptor will be word aligned.
     */
    size_t kAshmemSizePageAligned;
    if (bufferFd != -1) {
        // Allocate read counter and write counter only. User-supplied memory will be used for the
        // ringbuffer.
        kAshmemSizePageAligned = (kMetaDataSize + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    } else {
        // Allocate ringbuffer, read counter and write counter.
        kAshmemSizePageAligned = (hardware::details::alignToWordBoundary(kQueueSizeBytes) +
                                  kMetaDataSize + PAGE_SIZE - 1) &
                                 ~(PAGE_SIZE - 1);
    }

    /*
     * The native handle will contain the fds to be mapped.
     */
    int numFds = (bufferFd != -1) ? 2 : 1;
    native_handle_t* mqHandle = native_handle_create(numFds, 0 /* numInts */);
    if (mqHandle == nullptr) {
        return;
    }

    /*
     * Create an ashmem region to map the memory.
     */
    int ashmemFd = ashmem_create_region("MessageQueue", kAshmemSizePageAligned);
    ashmem_set_prot_region(ashmemFd, PROT_READ | PROT_WRITE);
    mqHandle->data[0] = ashmemFd;

    if (bufferFd != -1) {
        // Use user-supplied file descriptor for fdIndex 1
        mqHandle->data[1] = bufferFd.get();
        // release ownership of fd. mqHandle owns it now.
        if (bufferFd.release() < 0) {
            hardware::details::logError("Error releasing supplied bufferFd");
        }

        std::vector<android::hardware::GrantorDescriptor> grantors;
        grantors.resize(configureEventFlagWord ? hardware::details::kMinGrantorCountForEvFlagSupport
                                               : hardware::details::kMinGrantorCount);

        size_t memSize[] = {
                sizeof(hardware::details::RingBufferPosition), /* memory to be allocated for read
                                                                  pointer counter */
                sizeof(hardware::details::RingBufferPosition), /* memory to be allocated for write
                                                                  pointer counter */
                kQueueSizeBytes,              /* memory to be allocated for data buffer */
                sizeof(std::atomic<uint32_t>) /* memory to be allocated for EventFlag word */
        };

        for (size_t grantorPos = 0, offset = 0; grantorPos < grantors.size(); grantorPos++) {
            uint32_t grantorFdIndex;
            size_t grantorOffset;
            if (grantorPos == hardware::details::DATAPTRPOS) {
                grantorFdIndex = 1;
                grantorOffset = 0;
            } else {
                grantorFdIndex = 0;
                grantorOffset = offset;
                offset += memSize[grantorPos];
            }
            grantors[grantorPos] = {
                    0 /* grantor flags */, grantorFdIndex,
                    static_cast<uint32_t>(hardware::details::alignToWordBoundary(grantorOffset)),
                    memSize[grantorPos]};
        }

        mDesc = std::unique_ptr<Descriptor>(new (std::nothrow)
                                                    Descriptor(grantors, mqHandle, sizeof(T)));
    } else {
        mDesc = std::unique_ptr<Descriptor>(new (std::nothrow) Descriptor(
                kQueueSizeBytes, mqHandle, sizeof(T), configureEventFlagWord));
    }
    if (mDesc == nullptr) {
        native_handle_close(mqHandle);
        native_handle_delete(mqHandle);
        return;
    }
    initMemory(true);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
MessageQueueBase<MQDescriptorType, T, flavor>::~MessageQueueBase() {
    if (flavor == kUnsynchronizedWrite && mReadPtr != nullptr) {
        delete mReadPtr;
    } else if (mReadPtr != nullptr) {
        unmapGrantorDescr(mReadPtr, hardware::details::READPTRPOS);
    }
    if (mWritePtr != nullptr) {
        unmapGrantorDescr(mWritePtr, hardware::details::WRITEPTRPOS);
    }
    if (mRing != nullptr) {
        unmapGrantorDescr(mRing, hardware::details::DATAPTRPOS);
    }
    if (mEvFlagWord != nullptr) {
        unmapGrantorDescr(mEvFlagWord, hardware::details::EVFLAGWORDPOS);
        android::hardware::EventFlag::deleteEventFlag(&mEventFlag);
    }
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::write(const T* data) {
    return write(data, 1);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::read(T* data) {
    return read(data, 1);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::write(const T* data, size_t nMessages) {
    MemTransaction tx;
    return beginWrite(nMessages, &tx) && tx.copyTo(data, 0 /* startIdx */, nMessages) &&
           commitWrite(nMessages);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::writeBlocking(
        const T* data, size_t count, uint32_t readNotification, uint32_t writeNotification,
        int64_t timeOutNanos, android::hardware::EventFlag* evFlag) {
    static_assert(flavor == kSynchronizedReadWrite,
                  "writeBlocking can only be used with the "
                  "kSynchronizedReadWrite flavor.");
    /*
     * If evFlag is null and the FMQ does not have its own EventFlag object
     * return false;
     * If the flavor is kSynchronizedReadWrite and the readNotification
     * bit mask is zero return false;
     * If the count is greater than queue size, return false
     * to prevent blocking until timeOut.
     */
    if (evFlag == nullptr) {
        evFlag = mEventFlag;
        if (evFlag == nullptr) {
            hardware::details::logError(
                    "writeBlocking failed: called on MessageQueue with no Eventflag"
                    "configured or provided");
            return false;
        }
    }

    if (readNotification == 0 || (count > getQuantumCount())) {
        return false;
    }

    /*
     * There is no need to wait for a readNotification if there is sufficient
     * space to write is already present in the FMQ. The latter would be the case when
     * read operations read more number of messages than write operations write.
     * In other words, a single large read may clear the FMQ after multiple small
     * writes. This would fail to clear a pending readNotification bit since
     * EventFlag bits can only be cleared by a wait() call, however the bit would
     * be correctly cleared by the next writeBlocking() call.
     */

    bool result = write(data, count);
    if (result) {
        if (writeNotification) {
            evFlag->wake(writeNotification);
        }
        return result;
    }

    bool shouldTimeOut = timeOutNanos != 0;
    int64_t prevTimeNanos = shouldTimeOut ? android::elapsedRealtimeNano() : 0;

    while (true) {
        /* It is not required to adjust 'timeOutNanos' if 'shouldTimeOut' is false */
        if (shouldTimeOut) {
            /*
             * The current time and 'prevTimeNanos' are both CLOCK_BOOTTIME clock values(converted
             * to Nanoseconds)
             */
            int64_t currentTimeNs = android::elapsedRealtimeNano();
            /*
             * Decrement 'timeOutNanos' to account for the time taken to complete the last
             * iteration of the while loop.
             */
            timeOutNanos -= currentTimeNs - prevTimeNanos;
            prevTimeNanos = currentTimeNs;

            if (timeOutNanos <= 0) {
                /*
                 * Attempt write in case a context switch happened outside of
                 * evFlag->wait().
                 */
                result = write(data, count);
                break;
            }
        }

        /*
         * wait() will return immediately if there was a pending read
         * notification.
         */
        uint32_t efState = 0;
        status_t status = evFlag->wait(readNotification, &efState, timeOutNanos,
                                       true /* retry on spurious wake */);

        if (status != android::TIMED_OUT && status != android::NO_ERROR) {
            hardware::details::logError("Unexpected error code from EventFlag Wait status " +
                                        std::to_string(status));
            break;
        }

        if (status == android::TIMED_OUT) {
            break;
        }

        /*
         * If there is still insufficient space to write to the FMQ,
         * keep waiting for another readNotification.
         */
        if ((efState & readNotification) && write(data, count)) {
            result = true;
            break;
        }
    }

    if (result && writeNotification != 0) {
        evFlag->wake(writeNotification);
    }

    return result;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::writeBlocking(const T* data, size_t count,
                                                                  int64_t timeOutNanos) {
    return writeBlocking(data, count, FMQ_NOT_FULL, FMQ_NOT_EMPTY, timeOutNanos);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::readBlocking(
        T* data, size_t count, uint32_t readNotification, uint32_t writeNotification,
        int64_t timeOutNanos, android::hardware::EventFlag* evFlag) {
    static_assert(flavor == kSynchronizedReadWrite,
                  "readBlocking can only be used with the "
                  "kSynchronizedReadWrite flavor.");

    /*
     * If evFlag is null and the FMQ does not own its own EventFlag object
     * return false;
     * If the writeNotification bit mask is zero return false;
     * If the count is greater than queue size, return false to prevent
     * blocking until timeOut.
     */
    if (evFlag == nullptr) {
        evFlag = mEventFlag;
        if (evFlag == nullptr) {
            hardware::details::logError(
                    "readBlocking failed: called on MessageQueue with no Eventflag"
                    "configured or provided");
            return false;
        }
    }

    if (writeNotification == 0 || count > getQuantumCount()) {
        return false;
    }

    /*
     * There is no need to wait for a write notification if sufficient
     * data to read is already present in the FMQ. This would be the
     * case when read operations read lesser number of messages than
     * a write operation and multiple reads would be required to clear the queue
     * after a single write operation. This check would fail to clear a pending
     * writeNotification bit since EventFlag bits can only be cleared
     * by a wait() call, however the bit would be correctly cleared by the next
     * readBlocking() call.
     */

    bool result = read(data, count);
    if (result) {
        if (readNotification) {
            evFlag->wake(readNotification);
        }
        return result;
    }

    bool shouldTimeOut = timeOutNanos != 0;
    int64_t prevTimeNanos = shouldTimeOut ? android::elapsedRealtimeNano() : 0;

    while (true) {
        /* It is not required to adjust 'timeOutNanos' if 'shouldTimeOut' is false */
        if (shouldTimeOut) {
            /*
             * The current time and 'prevTimeNanos' are both CLOCK_BOOTTIME clock values(converted
             * to Nanoseconds)
             */
            int64_t currentTimeNs = android::elapsedRealtimeNano();
            /*
             * Decrement 'timeOutNanos' to account for the time taken to complete the last
             * iteration of the while loop.
             */
            timeOutNanos -= currentTimeNs - prevTimeNanos;
            prevTimeNanos = currentTimeNs;

            if (timeOutNanos <= 0) {
                /*
                 * Attempt read in case a context switch happened outside of
                 * evFlag->wait().
                 */
                result = read(data, count);
                break;
            }
        }

        /*
         * wait() will return immediately if there was a pending write
         * notification.
         */
        uint32_t efState = 0;
        status_t status = evFlag->wait(writeNotification, &efState, timeOutNanos,
                                       true /* retry on spurious wake */);

        if (status != android::TIMED_OUT && status != android::NO_ERROR) {
            hardware::details::logError("Unexpected error code from EventFlag Wait status " +
                                        std::to_string(status));
            break;
        }

        if (status == android::TIMED_OUT) {
            break;
        }

        /*
         * If the data in FMQ is still insufficient, go back to waiting
         * for another write notification.
         */
        if ((efState & writeNotification) && read(data, count)) {
            result = true;
            break;
        }
    }

    if (result && readNotification != 0) {
        evFlag->wake(readNotification);
    }
    return result;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::readBlocking(T* data, size_t count,
                                                                 int64_t timeOutNanos) {
    return readBlocking(data, count, FMQ_NOT_FULL, FMQ_NOT_EMPTY, timeOutNanos);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
size_t MessageQueueBase<MQDescriptorType, T, flavor>::availableToWriteBytes() const {
    return mDesc->getSize() - availableToReadBytes();
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
size_t MessageQueueBase<MQDescriptorType, T, flavor>::availableToWrite() const {
    return availableToWriteBytes() / sizeof(T);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
size_t MessageQueueBase<MQDescriptorType, T, flavor>::availableToRead() const {
    return availableToReadBytes() / sizeof(T);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::beginWrite(size_t nMessages,
                                                               MemTransaction* result) const {
    /*
     * If nMessages is greater than size of FMQ or in case of the synchronized
     * FMQ flavor, if there is not enough space to write nMessages, then return
     * result with null addresses.
     */
    if ((flavor == kSynchronizedReadWrite && (availableToWrite() < nMessages)) ||
        nMessages > getQuantumCount()) {
        *result = MemTransaction();
        return false;
    }

    auto writePtr = mWritePtr->load(std::memory_order_relaxed);
    if (writePtr % sizeof(T) != 0) {
        hardware::details::logError(
                "The write pointer has become misaligned. Writing to the queue is no longer "
                "possible.");
        hardware::details::errorWriteLog(0x534e4554, "184963385");
        return false;
    }
    size_t writeOffset = writePtr % mDesc->getSize();

    /*
     * From writeOffset, the number of messages that can be written
     * contiguously without wrapping around the ring buffer are calculated.
     */
    size_t contiguousMessages = (mDesc->getSize() - writeOffset) / sizeof(T);

    if (contiguousMessages < nMessages) {
        /*
         * Wrap around is required. Both result.first and result.second are
         * populated.
         */
        *result = MemTransaction(
                MemRegion(reinterpret_cast<T*>(mRing + writeOffset), contiguousMessages),
                MemRegion(reinterpret_cast<T*>(mRing), nMessages - contiguousMessages));
    } else {
        /*
         * A wrap around is not required to write nMessages. Only result.first
         * is populated.
         */
        *result = MemTransaction(MemRegion(reinterpret_cast<T*>(mRing + writeOffset), nMessages),
                                 MemRegion());
    }

    return true;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
/*
 * Disable integer sanitization since integer overflow here is allowed
 * and legal.
 */
__attribute__((no_sanitize("integer"))) bool
MessageQueueBase<MQDescriptorType, T, flavor>::commitWrite(size_t nMessages) {
    size_t nBytesWritten = nMessages * sizeof(T);
    auto writePtr = mWritePtr->load(std::memory_order_relaxed);
    writePtr += nBytesWritten;
    mWritePtr->store(writePtr, std::memory_order_release);
    /*
     * This method cannot fail now since we are only incrementing the writePtr
     * counter.
     */
    return true;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
size_t MessageQueueBase<MQDescriptorType, T, flavor>::availableToReadBytes() const {
    /*
     * This method is invoked by implementations of both read() and write() and
     * hence requires a memory_order_acquired load for both mReadPtr and
     * mWritePtr.
     */
    return mWritePtr->load(std::memory_order_acquire) - mReadPtr->load(std::memory_order_acquire);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::read(T* data, size_t nMessages) {
    MemTransaction tx;
    return beginRead(nMessages, &tx) && tx.copyFrom(data, 0 /* startIdx */, nMessages) &&
           commitRead(nMessages);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
/*
 * Disable integer sanitization since integer overflow here is allowed
 * and legal.
 */
__attribute__((no_sanitize("integer"))) bool
MessageQueueBase<MQDescriptorType, T, flavor>::beginRead(size_t nMessages,
                                                         MemTransaction* result) const {
    *result = MemTransaction();
    /*
     * If it is detected that the data in the queue was overwritten
     * due to the reader process being too slow, the read pointer counter
     * is set to the same as the write pointer counter to indicate error
     * and the read returns false;
     * Need acquire/release memory ordering for mWritePtr.
     */
    auto writePtr = mWritePtr->load(std::memory_order_acquire);
    /*
     * A relaxed load is sufficient for mReadPtr since there will be no
     * stores to mReadPtr from a different thread.
     */
    auto readPtr = mReadPtr->load(std::memory_order_relaxed);
    if (writePtr % sizeof(T) != 0 || readPtr % sizeof(T) != 0) {
        hardware::details::logError(
                "The write or read pointer has become misaligned. Reading from the queue is no "
                "longer possible.");
        hardware::details::errorWriteLog(0x534e4554, "184963385");
        return false;
    }

    if (writePtr - readPtr > mDesc->getSize()) {
        mReadPtr->store(writePtr, std::memory_order_release);
        return false;
    }

    size_t nBytesDesired = nMessages * sizeof(T);
    /*
     * Return if insufficient data to read in FMQ.
     */
    if (writePtr - readPtr < nBytesDesired) {
        return false;
    }

    size_t readOffset = readPtr % mDesc->getSize();
    /*
     * From readOffset, the number of messages that can be read contiguously
     * without wrapping around the ring buffer are calculated.
     */
    size_t contiguousMessages = (mDesc->getSize() - readOffset) / sizeof(T);

    if (contiguousMessages < nMessages) {
        /*
         * A wrap around is required. Both result.first and result.second
         * are populated.
         */
        *result = MemTransaction(
                MemRegion(reinterpret_cast<T*>(mRing + readOffset), contiguousMessages),
                MemRegion(reinterpret_cast<T*>(mRing), nMessages - contiguousMessages));
    } else {
        /*
         * A wrap around is not required. Only result.first need to be
         * populated.
         */
        *result = MemTransaction(MemRegion(reinterpret_cast<T*>(mRing + readOffset), nMessages),
                                 MemRegion());
    }

    return true;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
/*
 * Disable integer sanitization since integer overflow here is allowed
 * and legal.
 */
__attribute__((no_sanitize("integer"))) bool
MessageQueueBase<MQDescriptorType, T, flavor>::commitRead(size_t nMessages) {
    // TODO: Use a local copy of readPtr to avoid relazed mReadPtr loads.
    auto readPtr = mReadPtr->load(std::memory_order_relaxed);
    auto writePtr = mWritePtr->load(std::memory_order_acquire);
    /*
     * If the flavor is unsynchronized, it is possible that a write overflow may
     * have occurred between beginRead() and commitRead().
     */
    if (writePtr - readPtr > mDesc->getSize()) {
        mReadPtr->store(writePtr, std::memory_order_release);
        return false;
    }

    size_t nBytesRead = nMessages * sizeof(T);
    readPtr += nBytesRead;
    mReadPtr->store(readPtr, std::memory_order_release);
    return true;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
size_t MessageQueueBase<MQDescriptorType, T, flavor>::getQuantumSize() const {
    return mDesc->getQuantum();
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
size_t MessageQueueBase<MQDescriptorType, T, flavor>::getQuantumCount() const {
    return mDesc->getSize() / mDesc->getQuantum();
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
bool MessageQueueBase<MQDescriptorType, T, flavor>::isValid() const {
    return mRing != nullptr && mReadPtr != nullptr && mWritePtr != nullptr;
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
void* MessageQueueBase<MQDescriptorType, T, flavor>::mapGrantorDescr(uint32_t grantorIdx) {
    const native_handle_t* handle = mDesc->handle();
    auto grantors = mDesc->grantors();
    if (handle == nullptr) {
        hardware::details::logError("mDesc->handle is null");
        return nullptr;
    }

    if (grantorIdx >= grantors.size()) {
        hardware::details::logError(std::string("grantorIdx must be less than ") +
                                    std::to_string(grantors.size()));
        return nullptr;
    }

    int fdIndex = grantors[grantorIdx].fdIndex;
    /*
     * Offset for mmap must be a multiple of PAGE_SIZE.
     */
    int mapOffset = (grantors[grantorIdx].offset / PAGE_SIZE) * PAGE_SIZE;
    int mapLength = grantors[grantorIdx].offset - mapOffset + grantors[grantorIdx].extent;

    void* address = mmap(0, mapLength, PROT_READ | PROT_WRITE, MAP_SHARED, handle->data[fdIndex],
                         mapOffset);
    if (address == MAP_FAILED) {
        hardware::details::logError(std::string("mmap failed: ") + std::to_string(errno));
        return nullptr;
    }
    return reinterpret_cast<uint8_t*>(address) + (grantors[grantorIdx].offset - mapOffset);
}

template <template <typename, MQFlavor> typename MQDescriptorType, typename T, MQFlavor flavor>
void MessageQueueBase<MQDescriptorType, T, flavor>::unmapGrantorDescr(void* address,
                                                                      uint32_t grantorIdx) {
    auto grantors = mDesc->grantors();
    if ((address == nullptr) || (grantorIdx >= grantors.size())) {
        return;
    }

    int mapOffset = (grantors[grantorIdx].offset / PAGE_SIZE) * PAGE_SIZE;
    int mapLength = grantors[grantorIdx].offset - mapOffset + grantors[grantorIdx].extent;
    void* baseAddress =
            reinterpret_cast<uint8_t*>(address) - (grantors[grantorIdx].offset - mapOffset);
    if (baseAddress) munmap(baseAddress, mapLength);
}

}  // namespace hardware
