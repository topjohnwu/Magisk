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

#include <sys/mman.h>
#include <sys/stat.h>

#include <memory>

#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <android-base/unique_fd.h>
#include <android/cgrouprc.h>
#include <processgroup/processgroup.h>

#include "cgrouprc_internal.h"

using android::base::StringPrintf;
using android::base::unique_fd;

using android::cgrouprc::format::CgroupController;
using android::cgrouprc::format::CgroupFile;

static CgroupFile* LoadRcFile() {
    struct stat sb;

    unique_fd fd(TEMP_FAILURE_RETRY(open(CGROUPS_RC_PATH, O_RDONLY | O_CLOEXEC)));
    if (fd < 0) {
        PLOG(ERROR) << "open() failed for " << CGROUPS_RC_PATH;
        return nullptr;
    }

    if (fstat(fd, &sb) < 0) {
        PLOG(ERROR) << "fstat() failed for " << CGROUPS_RC_PATH;
        return nullptr;
    }

    size_t file_size = sb.st_size;
    if (file_size < sizeof(CgroupFile)) {
        LOG(ERROR) << "Invalid file format " << CGROUPS_RC_PATH;
        return nullptr;
    }

    CgroupFile* file_data = (CgroupFile*)mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (file_data == MAP_FAILED) {
        PLOG(ERROR) << "Failed to mmap " << CGROUPS_RC_PATH;
        return nullptr;
    }

    if (file_data->version_ != CgroupFile::FILE_CURR_VERSION) {
        LOG(ERROR) << CGROUPS_RC_PATH << " file version mismatch";
        munmap(file_data, file_size);
        return nullptr;
    }

    auto expected = sizeof(CgroupFile) + file_data->controller_count_ * sizeof(CgroupController);
    if (file_size != expected) {
        LOG(ERROR) << CGROUPS_RC_PATH << " file has invalid size, expected " << expected
                   << ", actual " << file_size;
        munmap(file_data, file_size);
        return nullptr;
    }

    return file_data;
}

static CgroupFile* GetInstance() {
    // Deliberately leak this object (not munmap) to avoid a race between destruction on
    // process exit and concurrent access from another thread.
    static auto* file = LoadRcFile();
    return file;
}

uint32_t ACgroupFile_getVersion() {
    auto file = GetInstance();
    if (file == nullptr) return 0;
    return file->version_;
}

uint32_t ACgroupFile_getControllerCount() {
    auto file = GetInstance();
    if (file == nullptr) return 0;
    return file->controller_count_;
}

const ACgroupController* ACgroupFile_getController(uint32_t index) {
    auto file = GetInstance();
    if (file == nullptr) return nullptr;
    CHECK(index < file->controller_count_);
    // Although the object is not actually an ACgroupController object, all ACgroupController_*
    // functions implicitly convert ACgroupController* back to CgroupController* before invoking
    // member functions.
    return static_cast<ACgroupController*>(&file->controllers_[index]);
}
