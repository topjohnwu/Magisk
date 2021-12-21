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

#define LOG_TAG "FMQ_EventFlags"

#include <linux/futex.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <limits>
#include <new>

#include <fmq/EventFlag.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>

namespace android {
namespace hardware {

status_t EventFlag::createEventFlag(int fd, off_t offset, EventFlag** flag) {
    if (flag == nullptr) {
        return BAD_VALUE;
    }

    status_t status = NO_MEMORY;
    *flag = nullptr;

    EventFlag* evFlag = new (std::nothrow) EventFlag(fd, offset, &status);
    if (evFlag != nullptr) {
        if (status == NO_ERROR) {
            *flag = evFlag;
        } else {
            delete evFlag;
        }
    }

    return status;
}

status_t EventFlag::createEventFlag(std::atomic<uint32_t>* fwAddr,
                                    EventFlag** flag) {
    if (flag == nullptr) {
        return BAD_VALUE;
    }

    status_t status = NO_MEMORY;
    *flag  = nullptr;

    EventFlag* evFlag = new (std::nothrow) EventFlag(fwAddr, &status);
    if (evFlag != nullptr) {
        if (status == NO_ERROR) {
            *flag = evFlag;
        } else {
            delete evFlag;
        }
    }

    return status;
}

/*
 * mmap memory for the futex word
 */
EventFlag::EventFlag(int fd, off_t offset, status_t* status) {
    mEfWordPtr = static_cast<std::atomic<uint32_t>*>(mmap(NULL,
                                                          sizeof(std::atomic<uint32_t>),
                                                          PROT_READ | PROT_WRITE,
                                                          MAP_SHARED, fd, offset));
    mEfWordNeedsUnmapping = true;
    if (mEfWordPtr != MAP_FAILED) {
        *status = NO_ERROR;
    } else {
        *status = -errno;
        ALOGE("Attempt to mmap event flag word failed: %s\n", strerror(errno));
    }
}

/*
 * Use this constructor if we already know where the futex word for
 * the EventFlag group lives.
 */
EventFlag::EventFlag(std::atomic<uint32_t>* fwAddr, status_t* status) {
    *status = NO_ERROR;
    if (fwAddr == nullptr) {
        *status = BAD_VALUE;
    } else {
        mEfWordPtr = fwAddr;
    }
}

/*
 * Set the specified bits of the futex word here and wake up any
 * thread waiting on any of the bits.
 */
status_t EventFlag::wake(uint32_t bitmask) {
    /*
     * Return early if there are no set bits in bitmask.
     */
    if (bitmask == 0) {
        return NO_ERROR;
    }

    status_t status = NO_ERROR;
    uint32_t old = std::atomic_fetch_or(mEfWordPtr, bitmask);
    /*
     * No need to call FUTEX_WAKE_BITSET if there were deferred wakes
     * already available for all set bits from bitmask.
     */
    constexpr size_t kIntMax = std::numeric_limits<int>::max();
    if ((~old & bitmask) != 0) {
        int ret = syscall(__NR_futex, mEfWordPtr, FUTEX_WAKE_BITSET, kIntMax, NULL, NULL, bitmask);
        if (ret == -1) {
            status = -errno;
            ALOGE("Error in event flag wake attempt: %s\n", strerror(errno));
        }
    }
    return status;
}

/*
 * Wait for any of the bits in the bitmask to be set
 * and return which bits caused the return.
 */
status_t EventFlag::waitHelper(uint32_t bitmask, uint32_t* efState, int64_t timeoutNanoSeconds) {
    /*
     * Return early if there are no set bits in bitmask.
     */
    if (bitmask == 0 || efState == nullptr) {
        return BAD_VALUE;
    }

    status_t status = NO_ERROR;
    uint32_t old = std::atomic_fetch_and(mEfWordPtr, ~bitmask);
    uint32_t setBits = old & bitmask;
    /*
     * If there was a deferred wake available, no need to call FUTEX_WAIT_BITSET.
     */
    if (setBits != 0) {
        *efState = setBits;
        return status;
    }

    uint32_t efWord = old & ~bitmask;
    /*
     * The syscall will put the thread to sleep only
     * if the futex word still contains the expected
     * value i.e. efWord. If the futex word contents have
     * changed, it fails with the error EAGAIN; If a timeout
     * is specified and exceeded the syscall fails with ETIMEDOUT.
     */
    int ret = 0;
    if (timeoutNanoSeconds) {
        struct timespec waitTimeAbsolute;
        addNanosecondsToCurrentTime(timeoutNanoSeconds, &waitTimeAbsolute);

        ret = syscall(__NR_futex, mEfWordPtr, FUTEX_WAIT_BITSET,
                      efWord, &waitTimeAbsolute, NULL, bitmask);
    } else {
        ret = syscall(__NR_futex, mEfWordPtr, FUTEX_WAIT_BITSET, efWord, NULL, NULL, bitmask);
    }
    if (ret == -1) {
        status = -errno;
        if (status != -EAGAIN && status != -ETIMEDOUT) {
            ALOGE("Event flag wait was unsuccessful: %s\n", strerror(errno));
        }
        *efState = 0;
    } else {
        old = std::atomic_fetch_and(mEfWordPtr, ~bitmask);
        *efState = old & bitmask;

        if (*efState == 0) {
            /* Return -EINTR for a spurious wakeup */
            status = -EINTR;
        }
    }
    return status;
}

/*
 * Wait for any of the bits in the bitmask to be set
 * and return which bits caused the return. If 'retry'
 * is true, wait again on a spurious wake-up.
 */
status_t EventFlag::wait(uint32_t bitmask,
                         uint32_t* efState,
                         int64_t timeoutNanoSeconds,
                         bool retry) {
    if (!retry) {
        return waitHelper(bitmask, efState, timeoutNanoSeconds);
    }

    bool shouldTimeOut = timeoutNanoSeconds != 0;
    int64_t prevTimeNs = shouldTimeOut ? android::elapsedRealtimeNano() : 0;
    status_t status;
    while (true) {
        if (shouldTimeOut) {
            int64_t currentTimeNs = android::elapsedRealtimeNano();
            /*
             * Decrement TimeOutNanos to account for the time taken to complete the last
             * iteration of the while loop.
             */
            timeoutNanoSeconds -= currentTimeNs - prevTimeNs;
            prevTimeNs = currentTimeNs;
            if (timeoutNanoSeconds <= 0) {
                status = -ETIMEDOUT;
                *efState = 0;
                break;
            }
        }

        status = waitHelper(bitmask, efState, timeoutNanoSeconds);
        if ((status != -EAGAIN) && (status != -EINTR)) {
            break;
        }
    }
    return status;
}

status_t EventFlag::unmapEventFlagWord(std::atomic<uint32_t>* efWordPtr,
                                       bool* efWordNeedsUnmapping) {
    status_t status = NO_ERROR;
    if (*efWordNeedsUnmapping) {
        int ret = munmap(efWordPtr, sizeof(std::atomic<uint32_t>));
        if (ret != 0) {
            status = -errno;
            ALOGE("Error in deleting event flag group: %s\n", strerror(errno));
        }
        *efWordNeedsUnmapping = false;
    }
    return status;
}

status_t EventFlag::deleteEventFlag(EventFlag** evFlag) {
    if (evFlag == nullptr || *evFlag == nullptr) {
        return BAD_VALUE;
    }

    status_t status = unmapEventFlagWord((*evFlag)->mEfWordPtr,
                                         &(*evFlag)->mEfWordNeedsUnmapping);
    delete *evFlag;
    *evFlag = nullptr;

    return status;
}

void EventFlag::addNanosecondsToCurrentTime(int64_t nanoSeconds, struct timespec* waitTime) {
    static constexpr int64_t kNanosPerSecond = 1000000000;

    clock_gettime(CLOCK_MONOTONIC, waitTime);
    waitTime->tv_sec += nanoSeconds / kNanosPerSecond;
    waitTime->tv_nsec += nanoSeconds % kNanosPerSecond;

    if (waitTime->tv_nsec >= kNanosPerSecond) {
        waitTime->tv_sec++;
        waitTime->tv_nsec -= kNanosPerSecond;
    }
}

EventFlag::~EventFlag() {
    unmapEventFlagWord(mEfWordPtr, &mEfWordNeedsUnmapping);
}

}  // namespace hardware
}  // namespace android
