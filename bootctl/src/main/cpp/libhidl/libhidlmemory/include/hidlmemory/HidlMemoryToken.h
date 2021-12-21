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
#ifndef ANDROID_HARDWARE_HIDLMEMORYTOKEN_H
#define ANDROID_HARDWARE_HIDLMEMORYTOKEN_H

#include <android/hidl/memory/token/1.0/IMemoryToken.h>

namespace android {
namespace hardware {

class HidlMemoryToken : public virtual ::android::hidl::memory::token::V1_0::IMemoryToken {
   public:
    Return<void> get(get_cb _hidl_cb) override;

    HidlMemoryToken(sp<HidlMemory> memory);

   protected:
    sp<HidlMemory> mMemory;
};

}  // namespace hardware
}  // namespace android
#endif
