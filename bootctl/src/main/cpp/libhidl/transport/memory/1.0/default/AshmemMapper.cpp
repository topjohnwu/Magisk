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

#include "AshmemMapper.h"

#include <inttypes.h>

#include <log/log.h>
#include <sys/mman.h>

#include "AshmemMemory.h"

namespace android {
namespace hidl {
namespace memory {
namespace V1_0 {
namespace implementation {

// Methods from ::android::hidl::memory::V1_0::IMapper follow.
Return<sp<IMemory>> AshmemMapper::mapMemory(const hidl_memory& mem) {
    if (mem.handle()->numFds == 0) {
        return nullptr;
    }

    // If ashmem service runs in 32-bit (size_t is uint32_t) and a 64-bit
    // client process requests a memory > 2^32 bytes, the size would be
    // converted to a 32-bit number in mmap. mmap could succeed but the
    // mapped memory's actual size would be smaller than the reported size.
    if (mem.size() > SIZE_MAX) {
        ALOGE("Cannot map %" PRIu64 " bytes of memory because it is too large.", mem.size());
        android_errorWriteLog(0x534e4554, "79376389");
        return nullptr;
    }

    int fd = mem.handle()->data[0];
    void* data = mmap(0, mem.size(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        // mmap never maps at address zero without MAP_FIXED, so we can avoid
        // exposing clients to MAP_FAILED.
        return nullptr;
    }

    return new AshmemMemory(mem, data);
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace memory
}  // namespace hidl
}  // namespace android
