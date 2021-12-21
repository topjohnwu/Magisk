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

#include <processgroup/format/cgroup_controller.h>

namespace android {
namespace cgrouprc {
namespace format {

CgroupController::CgroupController() : version_(0), flags_(0) {
    memset(name_, 0, sizeof(name_));
    memset(path_, 0, sizeof(path_));
}

CgroupController::CgroupController(uint32_t version, uint32_t flags, const std::string& name,
                                   const std::string& path)
    : CgroupController() {
    // strlcpy isn't available on host. Although there is an implementation
    // in licutils, libcutils itself depends on libcgrouprc_format, causing
    // a circular dependency.
    version_ = version;
    flags_ = flags;
    strncpy(name_, name.c_str(), sizeof(name_) - 1);
    name_[sizeof(name_) - 1] = '\0';
    strncpy(path_, path.c_str(), sizeof(path_) - 1);
    path_[sizeof(path_) - 1] = '\0';
}

uint32_t CgroupController::version() const {
    return version_;
}

uint32_t CgroupController::flags() const {
    return flags_;
}

const char* CgroupController::name() const {
    return name_;
}

const char* CgroupController::path() const {
    return path_;
}

void CgroupController::set_flags(uint32_t flags) {
    flags_ = flags;
}

}  // namespace format
}  // namespace cgrouprc
}  // namespace android
