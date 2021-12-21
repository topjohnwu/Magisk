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

#include <stdint.h>
#include <string>

namespace android {
namespace cgrouprc {
namespace format {

// Minimal controller description to be mmapped into process address space
struct CgroupController {
  public:
    CgroupController();
    CgroupController(uint32_t version, uint32_t flags, const std::string& name,
                     const std::string& path);

    uint32_t version() const;
    uint32_t flags() const;
    const char* name() const;
    const char* path() const;

    void set_flags(uint32_t flags);

  private:
    static constexpr size_t CGROUP_NAME_BUF_SZ = 16;
    static constexpr size_t CGROUP_PATH_BUF_SZ = 32;

    uint32_t version_;
    uint32_t flags_;
    char name_[CGROUP_NAME_BUF_SZ];
    char path_[CGROUP_PATH_BUF_SZ];
};

}  // namespace format
}  // namespace cgrouprc
}  // namespace android
