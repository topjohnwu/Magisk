/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <gtest/gtest.h>

namespace android {
namespace hardware {

static inline std::string Sanitize(std::string name) {
    for (size_t i = 0; i < name.size(); i++) {
        // gtest test names must only contain alphanumeric characters
        if (!std::isalnum(name[i])) name[i] = '_';
    }
    return name;
}

static inline std::string PrintInstanceNameToString(
        const testing::TestParamInfo<std::string>& info) {
    // test names need to be unique -> index prefix
    std::string name = std::to_string(info.index) + "/" + info.param;
    return Sanitize(name);
}

template <typename... T>
static inline std::string PrintInstanceTupleNameToString(
        const testing::TestParamInfo<std::tuple<T...>>& info) {
    std::vector<std::string> instances = std::apply(
            [](auto&&... elems) {
                std::vector<std::string> instances;
                instances.reserve(sizeof...(elems));
                (instances.push_back(std::forward<decltype(elems)>(elems)), ...);
                return instances;
            },
            info.param);
    std::string param_string;
    for (const std::string& instance : instances) {
        param_string += instance + "_";
    }
    param_string += std::to_string(info.index);

    return Sanitize(param_string);
}

}  // namespace hardware
}  // namespace android
