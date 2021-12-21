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

#define LOG_TAG "AshmemAllocator"
#include <android-base/logging.h>

#include "AshmemAllocator.h"

#include <cutils/ashmem.h>

namespace android {
namespace hidl {
namespace allocator {
namespace V1_0 {
namespace implementation {

static hidl_memory allocateOne(uint64_t size) {
    int fd = ashmem_create_region("AshmemAllocator_hidl", size);
    if (fd < 0) {
        LOG(WARNING) << "ashmem_create_region(" << size << ") fails with " << fd;
        return hidl_memory();
    }

    native_handle_t* handle = native_handle_create(1, 0);
    handle->data[0] = fd;
    LOG(VERBOSE) << "ashmem_create_region(" << size << ") returning hidl_memory(" << handle << ", "
                 << size << ")";
    return hidl_memory("ashmem", handle, size);
}

static void cleanup(hidl_memory&& memory) {
    if (memory.handle() == nullptr) {
        return;
    }

    native_handle_close(const_cast<native_handle_t *>(memory.handle()));
    native_handle_delete(const_cast<native_handle_t *>(memory.handle()));
}

Return<void> AshmemAllocator::allocate(uint64_t size, allocate_cb _hidl_cb) {
    hidl_memory memory = allocateOne(size);
    _hidl_cb(memory.handle() != nullptr /* success */, memory);
    cleanup(std::move(memory));

    return Void();
}

Return<void> AshmemAllocator::batchAllocate(uint64_t size, uint64_t count, batchAllocate_cb _hidl_cb) {
    // resize fails if count > 2^32
    if (count > UINT32_MAX) {
        _hidl_cb(false /* success */, {});
        return Void();
    }

    hidl_vec<hidl_memory> batch;
    batch.resize(count);

    uint64_t allocated;
    for (allocated = 0; allocated < count; allocated++) {
        batch[allocated] = allocateOne(size);

        if (batch[allocated].handle() == nullptr) {
            LOG(WARNING) << "batchAllocate(" << size << ", " << count << ") fails @ #" << allocated;
            break;
        }
    }

    // batch[i].handle() != nullptr for i in [0, allocated - 1].
    // batch[i].handle() == nullptr for i in [allocated, count - 1].

    if (allocated < count) {
        _hidl_cb(false /* success */, {});
    } else {
        _hidl_cb(true /* success */, batch);
    }

    for (uint64_t i = 0; i < allocated; i++) {
        cleanup(std::move(batch[i]));
    }

    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace allocator
}  // namespace hidl
}  // namespace android
