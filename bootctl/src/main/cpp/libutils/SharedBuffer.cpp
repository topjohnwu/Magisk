/*
 * Copyright (C) 2005 The Android Open Source Project
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

#define LOG_TAG "sharedbuffer"

#include "SharedBuffer.h"

#include <stdlib.h>
#include <string.h>

#include <log/log.h>

// ---------------------------------------------------------------------------

namespace android {

SharedBuffer* SharedBuffer::alloc(size_t size)
{
    // Don't overflow if the combined size of the buffer / header is larger than
    // size_max.
    LOG_ALWAYS_FATAL_IF((size >= (SIZE_MAX - sizeof(SharedBuffer))),
                        "Invalid buffer size %zu", size);

    SharedBuffer* sb = static_cast<SharedBuffer *>(malloc(sizeof(SharedBuffer) + size));
    if (sb) {
        // Should be std::atomic_init(&sb->mRefs, 1);
        // But that generates a warning with some compilers.
        // The following is OK on Android-supported platforms.
        sb->mRefs.store(1, std::memory_order_relaxed);
        sb->mSize = size;
        sb->mClientMetadata = 0;
    }
    return sb;
}


void SharedBuffer::dealloc(const SharedBuffer* released)
{
    free(const_cast<SharedBuffer*>(released));
}

SharedBuffer* SharedBuffer::edit() const
{
    if (onlyOwner()) {
        return const_cast<SharedBuffer*>(this);
    }
    SharedBuffer* sb = alloc(mSize);
    if (sb) {
        memcpy(sb->data(), data(), size());
        release();
    }
    return sb;
}

SharedBuffer* SharedBuffer::editResize(size_t newSize) const
{
    if (onlyOwner()) {
        SharedBuffer* buf = const_cast<SharedBuffer*>(this);
        if (buf->mSize == newSize) return buf;
        // Don't overflow if the combined size of the new buffer / header is larger than
        // size_max.
        LOG_ALWAYS_FATAL_IF((newSize >= (SIZE_MAX - sizeof(SharedBuffer))),
                            "Invalid buffer size %zu", newSize);

        buf = (SharedBuffer*)realloc(buf, sizeof(SharedBuffer) + newSize);
        if (buf != nullptr) {
            buf->mSize = newSize;
            return buf;
        }
    }
    SharedBuffer* sb = alloc(newSize);
    if (sb) {
        const size_t mySize = mSize;
        memcpy(sb->data(), data(), newSize < mySize ? newSize : mySize);
        release();
    }
    return sb;    
}

SharedBuffer* SharedBuffer::attemptEdit() const
{
    if (onlyOwner()) {
        return const_cast<SharedBuffer*>(this);
    }
    return nullptr;
}

SharedBuffer* SharedBuffer::reset(size_t new_size) const
{
    // cheap-o-reset.
    SharedBuffer* sb = alloc(new_size);
    if (sb) {
        release();
    }
    return sb;
}

void SharedBuffer::acquire() const {
    mRefs.fetch_add(1, std::memory_order_relaxed);
}

int32_t SharedBuffer::release(uint32_t flags) const
{
    const bool useDealloc = ((flags & eKeepStorage) == 0);
    if (onlyOwner()) {
        // Since we're the only owner, our reference count goes to zero.
        mRefs.store(0, std::memory_order_relaxed);
        if (useDealloc) {
            dealloc(this);
        }
        // As the only owner, our previous reference count was 1.
        return 1;
    }
    // There's multiple owners, we need to use an atomic decrement.
    int32_t prevRefCount = mRefs.fetch_sub(1, std::memory_order_release);
    if (prevRefCount == 1) {
        // We're the last reference, we need the acquire fence.
        atomic_thread_fence(std::memory_order_acquire);
        if (useDealloc) {
            dealloc(this);
        }
    }
    return prevRefCount;
}


}; // namespace android
