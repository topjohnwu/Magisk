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

#include <assert.h>
#include <string>

namespace android {
namespace hardware {

enum MQFlavor : uint32_t {
    /*
     * kSynchronizedReadWrite represents the wait-free synchronized flavor of the
     * FMQ. It is intended to be have a single reader and single writer.
     * Attempts to overflow/underflow returns a failure.
     */
    kSynchronizedReadWrite = 0x01,
    /*
     * kUnsynchronizedWrite represents the flavor of FMQ where writes always
     * succeed. This flavor allows one writer and many readers. A read operation
     * can detect an overwrite and reset the read counter.
     */
    kUnsynchronizedWrite = 0x02
};

struct GrantorDescriptor {
    uint32_t flags __attribute__((aligned(4)));
    uint32_t fdIndex __attribute__((aligned(4)));
    uint32_t offset __attribute__((aligned(4)));
    uint64_t extent __attribute__((aligned(8)));
};

static_assert(offsetof(GrantorDescriptor, flags) == 0, "wrong offset");
static_assert(offsetof(GrantorDescriptor, fdIndex) == 4, "wrong offset");
static_assert(offsetof(GrantorDescriptor, offset) == 8, "wrong offset");
static_assert(offsetof(GrantorDescriptor, extent) == 16, "wrong offset");
static_assert(sizeof(GrantorDescriptor) == 24, "wrong size");
static_assert(__alignof(GrantorDescriptor) == 8, "wrong alignment");

namespace details {

void logError(const std::string& message);
void errorWriteLog(int tag, const char* message);

typedef uint64_t RingBufferPosition;
enum GrantorType : int { READPTRPOS = 0, WRITEPTRPOS, DATAPTRPOS, EVFLAGWORDPOS };
/*
 * There should at least be GrantorDescriptors for the read counter, write
 * counter and data buffer. A GrantorDescriptor for an EventFlag word is
 * not required if there is no need for blocking FMQ operations.
 */
static constexpr int32_t kMinGrantorCount = DATAPTRPOS + 1;

/*
 * Minimum number of GrantorDescriptors required if EventFlag support is
 * needed for blocking FMQ operations.
 */
static constexpr int32_t kMinGrantorCountForEvFlagSupport = EVFLAGWORDPOS + 1;

static inline size_t alignToWordBoundary(size_t length) {
    constexpr size_t kAlignmentSize = 64;
    if (kAlignmentSize % __WORDSIZE != 0) {
#ifdef __BIONIC__
        __assert(__FILE__, __LINE__, "Incompatible word size");
#endif
    }

    /*
     * Check if alignment to word boundary would cause an overflow.
     */
    if (length > SIZE_MAX - kAlignmentSize / 8 + 1) {
#ifdef __BIONIC__
        __assert(__FILE__, __LINE__, "Queue size too large");
#endif
    }

    return (length + kAlignmentSize / 8 - 1) & ~(kAlignmentSize / 8 - 1U);
}

static inline size_t isAlignedToWordBoundary(size_t offset) {
    constexpr size_t kAlignmentSize = 64;
    return (offset & (kAlignmentSize / 8 - 1)) == 0;
}

}  // namespace details
}  // namespace hardware
}  // namespace android
