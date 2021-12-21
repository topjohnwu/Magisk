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

//#define LOG_NDEBUG 0
#define LOG_TAG "libprocessgroup"

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <regex>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android-base/unique_fd.h>
#include <cgroup_map.h>
#include <json/reader.h>
#include <json/value.h>
#include <processgroup/processgroup.h>

using android::base::GetBoolProperty;
using android::base::StringPrintf;
using android::base::unique_fd;

static constexpr const char* CGROUP_PROCS_FILE = "/cgroup.procs";
static constexpr const char* CGROUP_TASKS_FILE = "/tasks";
static constexpr const char* CGROUP_TASKS_FILE_V2 = "/cgroup.tasks";

uint32_t CgroupController::version() const {
    CHECK(HasValue());
    return ACgroupController_getVersion(controller_);
}

const char* CgroupController::name() const {
    CHECK(HasValue());
    return ACgroupController_getName(controller_);
}

const char* CgroupController::path() const {
    CHECK(HasValue());
    return ACgroupController_getPath(controller_);
}

bool CgroupController::HasValue() const {
    return controller_ != nullptr;
}

bool CgroupController::IsUsable() {
    if (!HasValue()) return false;

    if (state_ == UNKNOWN) {
        if (__builtin_available(android 30, *)) {
            uint32_t flags = ACgroupController_getFlags(controller_);
            state_ = (flags & CGROUPRC_CONTROLLER_FLAG_MOUNTED) != 0 ? USABLE : MISSING;
        } else {
            state_ = access(GetProcsFilePath("", 0, 0).c_str(), F_OK) == 0 ? USABLE : MISSING;
        }
    }

    return state_ == USABLE;
}

std::string CgroupController::GetTasksFilePath(const std::string& rel_path) const {
    std::string tasks_path = path();

    if (!rel_path.empty()) {
        tasks_path += "/" + rel_path;
    }
    return (version() == 1) ? tasks_path + CGROUP_TASKS_FILE : tasks_path + CGROUP_TASKS_FILE_V2;
}

std::string CgroupController::GetProcsFilePath(const std::string& rel_path, uid_t uid,
                                               pid_t pid) const {
    std::string proc_path(path());
    proc_path.append("/").append(rel_path);
    proc_path = regex_replace(proc_path, std::regex("<uid>"), std::to_string(uid));
    proc_path = regex_replace(proc_path, std::regex("<pid>"), std::to_string(pid));

    return proc_path.append(CGROUP_PROCS_FILE);
}

bool CgroupController::GetTaskGroup(int tid, std::string* group) const {
    std::string file_name = StringPrintf("/proc/%d/cgroup", tid);
    std::string content;
    if (!android::base::ReadFileToString(file_name, &content)) {
        PLOG(ERROR) << "Failed to read " << file_name;
        return false;
    }

    // if group is null and tid exists return early because
    // user is not interested in cgroup membership
    if (group == nullptr) {
        return true;
    }

    std::string cg_tag;

    if (version() == 2) {
        cg_tag = "0::";
    } else {
        cg_tag = StringPrintf(":%s:", name());
    }
    size_t start_pos = content.find(cg_tag);
    if (start_pos == std::string::npos) {
        return false;
    }

    start_pos += cg_tag.length() + 1;  // skip '/'
    size_t end_pos = content.find('\n', start_pos);
    if (end_pos == std::string::npos) {
        *group = content.substr(start_pos, std::string::npos);
    } else {
        *group = content.substr(start_pos, end_pos - start_pos);
    }

    return true;
}

CgroupMap::CgroupMap() {
    if (!LoadRcFile()) {
        LOG(ERROR) << "CgroupMap::LoadRcFile called for [" << getpid() << "] failed";
    }
}

CgroupMap& CgroupMap::GetInstance() {
    // Deliberately leak this object to avoid a race between destruction on
    // process exit and concurrent access from another thread.
    static auto* instance = new CgroupMap;
    return *instance;
}

bool CgroupMap::LoadRcFile() {
    if (!loaded_) {
        loaded_ = (ACgroupFile_getVersion() != 0);
    }
    return loaded_;
}

void CgroupMap::Print() const {
    if (!loaded_) {
        LOG(ERROR) << "CgroupMap::Print called for [" << getpid()
                   << "] failed, RC file was not initialized properly";
        return;
    }
    LOG(INFO) << "File version = " << ACgroupFile_getVersion();
    LOG(INFO) << "File controller count = " << ACgroupFile_getControllerCount();

    LOG(INFO) << "Mounted cgroups:";

    auto controller_count = ACgroupFile_getControllerCount();
    for (uint32_t i = 0; i < controller_count; ++i) {
        const ACgroupController* controller = ACgroupFile_getController(i);
        if (__builtin_available(android 30, *)) {
            LOG(INFO) << "\t" << ACgroupController_getName(controller) << " ver "
                      << ACgroupController_getVersion(controller) << " path "
                      << ACgroupController_getPath(controller) << " flags "
                      << ACgroupController_getFlags(controller);
        } else {
            LOG(INFO) << "\t" << ACgroupController_getName(controller) << " ver "
                      << ACgroupController_getVersion(controller) << " path "
                      << ACgroupController_getPath(controller);
        }
    }
}

CgroupController CgroupMap::FindController(const std::string& name) const {
    if (!loaded_) {
        LOG(ERROR) << "CgroupMap::FindController called for [" << getpid()
                   << "] failed, RC file was not initialized properly";
        return CgroupController(nullptr);
    }

    auto controller_count = ACgroupFile_getControllerCount();
    for (uint32_t i = 0; i < controller_count; ++i) {
        const ACgroupController* controller = ACgroupFile_getController(i);
        if (name == ACgroupController_getName(controller)) {
            return CgroupController(controller);
        }
    }

    return CgroupController(nullptr);
}
