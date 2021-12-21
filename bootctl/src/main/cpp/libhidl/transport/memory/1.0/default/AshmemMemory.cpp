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

#include <sys/mman.h>

#include "AshmemMemory.h"

namespace android {
namespace hidl {
namespace memory {
namespace V1_0 {
namespace implementation {

AshmemMemory::AshmemMemory(const hidl_memory& memory, void* data)
  : mMemory(memory),
    mData(data)
{}

AshmemMemory::~AshmemMemory()
{
    // TODO: Move implementation to mapper class
    munmap(mData, mMemory.size());
}

// Methods from ::android::hidl::memory::V1_0::IMemory follow.
Return<void> AshmemMemory::update() {
    // NOOP (since non-remoted memory)
    return Void();
}

Return<void> AshmemMemory::updateRange(uint64_t /* start */, uint64_t /* length */) {
    // NOOP (since non-remoted memory)
    return Void();
}

Return<void> AshmemMemory::read() {
    // NOOP (since non-remoted memory)
    return Void();
}

Return<void> AshmemMemory::readRange(uint64_t /* start */, uint64_t /* length */) {
    // NOOP (since non-remoted memory)
    return Void();
}

Return<void> AshmemMemory::commit() {
    // NOOP (since non-remoted memory)
    return Void();
}

Return<void*> AshmemMemory::getPointer() {
    return mData;
}

Return<uint64_t> AshmemMemory::getSize() {
    return mMemory.size();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace memory
}  // namespace hidl
}  // namespace android
