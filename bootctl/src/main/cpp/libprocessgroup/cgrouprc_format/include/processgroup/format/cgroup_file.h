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

#pragma once

#include <processgroup/format/cgroup_controller.h>

namespace android {
namespace cgrouprc {
namespace format {

struct CgroupFile {
    uint32_t version_;
    uint32_t controller_count_;
    CgroupController controllers_[];

    static constexpr uint32_t FILE_VERSION_1 = 1;
    static constexpr uint32_t FILE_CURR_VERSION = FILE_VERSION_1;
};

}  // namespace format
}  // namespace cgrouprc
}  // namespace android
