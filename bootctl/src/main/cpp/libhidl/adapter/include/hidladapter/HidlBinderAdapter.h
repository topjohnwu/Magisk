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

#include <android/hidl/base/1.0/IBase.h>
#include <hidl/HidlSupport.h>

#include <map>

namespace android {
namespace hardware {

namespace details {

using IBase = ::android::hidl::base::V1_0::IBase;

// AdapterFactory(impl) -> adapter
using AdapterFactory = std::function<sp<IBase>(sp<IBase>)>;
// AdaptersFactory(package@interface)(impl) -> adapter
using AdaptersFactory = std::map<std::string, AdapterFactory>;

int adapterMain(const std::string& package, int argc, char** argv, const AdaptersFactory& adapters);

sp<IBase> adaptWithDefault(const sp<IBase>& something,
                           const std::function<sp<IBase>()>& makeDefault);

}  // namespace details

template <typename... Adapters>
int adapterMain(const std::string& package, int argc, char** argv) {
    return details::adapterMain(
        package, argc, argv,
        {{Adapters::Pure::descriptor, [](sp<::android::hidl::base::V1_0::IBase> base) {
              return details::adaptWithDefault(
                  base, [&] { return new Adapters(Adapters::Pure::castFrom(base)); });
          }}...});
}

}  // namespace hardware
}  // namespace android