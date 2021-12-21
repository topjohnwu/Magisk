/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <unistd.h>

#include <hidlmemory/FrameworkUtils.h>

namespace android {

namespace hardware {

sp<HidlMemory> fromHeap(const sp<IMemoryHeap>& heap) {
    int fd = dup(heap->getHeapID());

    if (fd < 0) {
        return HidlMemory::getInstance(hidl_memory());
    }

    // Only being used because this library is paired with the IAllocator
    // ashmem. Other places should not make assumptions about the contents
    // of this memory.
    return HidlMemory::getInstance("ashmem", fd, heap->getSize());
}

}  // namespace hardware
}  // namespace android
