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
#define LOG_TAG "libhidlmemory"

#include <map>
#include <mutex>
#include <string>

#include <hidlmemory/mapping.h>

#include <android-base/logging.h>
#include <android/hidl/memory/1.0/IMapper.h>
#include <hidl/HidlSupport.h>
#include <log/log.h>

using android::sp;
using android::hidl::memory::V1_0::IMemory;
using android::hidl::memory::V1_0::IMapper;

namespace android {
namespace hardware {

static std::map<std::string, sp<IMapper>> gMappersByName;
static std::mutex gMutex;

static inline sp<IMapper> getMapperService(const std::string& name) {
    std::unique_lock<std::mutex> _lock(gMutex);
    auto iter = gMappersByName.find(name);
    if (iter != gMappersByName.end()) {
        return iter->second;
    }

    sp<IMapper> mapper = IMapper::getService(name, true /* getStub */);
    if (mapper != nullptr) {
        gMappersByName[name] = mapper;
    }
    return mapper;
}

sp<IMemory> mapMemory(const hidl_memory& memory) {

    sp<IMapper> mapper = getMapperService(memory.name());

    if (mapper == nullptr) {
        LOG(ERROR) << "Could not fetch mapper for " << memory.name() << " shared memory";
        return nullptr;
    }

    if (mapper->isRemote()) {
        LOG(ERROR) << "IMapper must be a passthrough service.";
        return nullptr;
    }

    // hidl_memory's size is stored in uint64_t, but mapMemory's mmap will map
    // size in size_t. If size is over SIZE_MAX, mapMemory could succeed
    // but the mapped memory's actual size will be smaller than the reported size.
    if (memory.size() > SIZE_MAX) {
        LOG(ERROR) << "Cannot map " << memory.size() << " bytes of memory because it is too large.";
        android_errorWriteLog(0x534e4554, "79376389");
        return nullptr;
    }

    Return<sp<IMemory>> ret = mapper->mapMemory(memory);

    if (!ret.isOk()) {
        LOG(ERROR) << "hidl_memory map returned transport error.";
        return nullptr;
    }

    return ret;
}

}  // namespace hardware
}  // namespace android
