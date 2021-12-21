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

#include <dirent.h>
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
#include <android/cgrouprc.h>
#include <json/reader.h>
#include <json/value.h>
#include <processgroup/format/cgroup_file.h>
#include <processgroup/processgroup.h>
#include <processgroup/setup.h>

#include "cgroup_descriptor.h"

using android::base::GetUintProperty;
using android::base::StringPrintf;
using android::base::unique_fd;

namespace android {
namespace cgrouprc {

static constexpr const char* CGROUPS_DESC_FILE = "/etc/cgroups.json";
static constexpr const char* CGROUPS_DESC_VENDOR_FILE = "/vendor/etc/cgroups.json";

static constexpr const char* TEMPLATE_CGROUPS_DESC_API_FILE = "/etc/task_profiles/cgroups_%u.json";

static bool ChangeDirModeAndOwner(const std::string& path, mode_t mode, const std::string& uid,
                                  const std::string& gid, bool permissive_mode = false) {
    uid_t pw_uid = -1;
    gid_t gr_gid = -1;

    if (!uid.empty()) {
        passwd* uid_pwd = getpwnam(uid.c_str());
        if (!uid_pwd) {
            PLOG(ERROR) << "Unable to decode UID for '" << uid << "'";
            return false;
        }

        pw_uid = uid_pwd->pw_uid;
        gr_gid = -1;

        if (!gid.empty()) {
            group* gid_pwd = getgrnam(gid.c_str());
            if (!gid_pwd) {
                PLOG(ERROR) << "Unable to decode GID for '" << gid << "'";
                return false;
            }
            gr_gid = gid_pwd->gr_gid;
        }
    }

    auto dir = std::unique_ptr<DIR, decltype(&closedir)>(opendir(path.c_str()), closedir);

    if (dir == NULL) {
        PLOG(ERROR) << "opendir failed for " << path;
        return false;
    }

    struct dirent* dir_entry;
    while ((dir_entry = readdir(dir.get()))) {
        if (!strcmp("..", dir_entry->d_name)) {
            continue;
        }

        std::string file_path = path + "/" + dir_entry->d_name;

        if (pw_uid != -1 && lchown(file_path.c_str(), pw_uid, gr_gid) < 0) {
            PLOG(ERROR) << "lchown() failed for " << file_path;
            return false;
        }

        if (fchmodat(AT_FDCWD, file_path.c_str(), mode, AT_SYMLINK_NOFOLLOW) != 0 &&
            (errno != EROFS || !permissive_mode)) {
            PLOG(ERROR) << "fchmodat() failed for " << path;
            return false;
        }
    }

    return true;
}

static bool Mkdir(const std::string& path, mode_t mode, const std::string& uid,
                  const std::string& gid) {
    bool permissive_mode = false;

    if (mode == 0) {
        /* Allow chmod to fail */
        permissive_mode = true;
        mode = 0755;
    }

    if (mkdir(path.c_str(), mode) != 0) {
        // /acct is a special case when the directory already exists
        if (errno != EEXIST) {
            PLOG(ERROR) << "mkdir() failed for " << path;
            return false;
        } else {
            permissive_mode = true;
        }
    }

    if (uid.empty() && permissive_mode) {
        return true;
    }

    if (!ChangeDirModeAndOwner(path, mode, uid, gid, permissive_mode)) {
        PLOG(ERROR) << "change of ownership or mode failed for " << path;
        return false;
    }

    return true;
}

static void MergeCgroupToDescriptors(std::map<std::string, CgroupDescriptor>* descriptors,
                                     const Json::Value& cgroup, const std::string& name,
                                     const std::string& root_path, int cgroups_version) {
    std::string path;

    if (!root_path.empty()) {
        path = root_path + "/" + cgroup["Path"].asString();
    } else {
        path = cgroup["Path"].asString();
    }

    uint32_t controller_flags = 0;

    if (cgroup["NeedsActivation"].isBool() && cgroup["NeedsActivation"].asBool()) {
        controller_flags |= CGROUPRC_CONTROLLER_FLAG_NEEDS_ACTIVATION;
    }

    if (cgroup["Optional"].isBool() && cgroup["Optional"].asBool()) {
        controller_flags |= CGROUPRC_CONTROLLER_FLAG_OPTIONAL;
    }

    CgroupDescriptor descriptor(
            cgroups_version, name, path, std::strtoul(cgroup["Mode"].asString().c_str(), 0, 8),
            cgroup["UID"].asString(), cgroup["GID"].asString(), controller_flags);

    auto iter = descriptors->find(name);
    if (iter == descriptors->end()) {
        descriptors->emplace(name, descriptor);
    } else {
        iter->second = descriptor;
    }
}

static bool ReadDescriptorsFromFile(const std::string& file_name,
                                    std::map<std::string, CgroupDescriptor>* descriptors) {
    std::vector<CgroupDescriptor> result;
    std::string json_doc;

    if (!android::base::ReadFileToString(file_name, &json_doc)) {
        PLOG(ERROR) << "Failed to read task profiles from " << file_name;
        return false;
    }

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    std::string errorMessage;
    if (!reader->parse(&*json_doc.begin(), &*json_doc.end(), &root, &errorMessage)) {
        LOG(ERROR) << "Failed to parse cgroups description: " << errorMessage;
        return false;
    }

    if (root.isMember("Cgroups")) {
        const Json::Value& cgroups = root["Cgroups"];
        for (Json::Value::ArrayIndex i = 0; i < cgroups.size(); ++i) {
            std::string name = cgroups[i]["Controller"].asString();
            MergeCgroupToDescriptors(descriptors, cgroups[i], name, "", 1);
        }
    }

    if (root.isMember("Cgroups2")) {
        const Json::Value& cgroups2 = root["Cgroups2"];
        std::string root_path = cgroups2["Path"].asString();
        MergeCgroupToDescriptors(descriptors, cgroups2, CGROUPV2_CONTROLLER_NAME, "", 2);

        const Json::Value& childGroups = cgroups2["Controllers"];
        for (Json::Value::ArrayIndex i = 0; i < childGroups.size(); ++i) {
            std::string name = childGroups[i]["Controller"].asString();
            MergeCgroupToDescriptors(descriptors, childGroups[i], name, root_path, 2);
        }
    }

    return true;
}

static bool ReadDescriptors(std::map<std::string, CgroupDescriptor>* descriptors) {
    // load system cgroup descriptors
    if (!ReadDescriptorsFromFile(CGROUPS_DESC_FILE, descriptors)) {
        return false;
    }

    // load API-level specific system cgroups descriptors if available
    unsigned int api_level = GetUintProperty<unsigned int>("ro.product.first_api_level", 0);
    if (api_level > 0) {
        std::string api_cgroups_path =
                android::base::StringPrintf(TEMPLATE_CGROUPS_DESC_API_FILE, api_level);
        if (!access(api_cgroups_path.c_str(), F_OK) || errno != ENOENT) {
            if (!ReadDescriptorsFromFile(api_cgroups_path, descriptors)) {
                return false;
            }
        }
    }

    // load vendor cgroup descriptors if the file exists
    if (!access(CGROUPS_DESC_VENDOR_FILE, F_OK) &&
        !ReadDescriptorsFromFile(CGROUPS_DESC_VENDOR_FILE, descriptors)) {
        return false;
    }

    return true;
}

// To avoid issues in sdk_mac build
#if defined(__ANDROID__)

static bool SetupCgroup(const CgroupDescriptor& descriptor) {
    const format::CgroupController* controller = descriptor.controller();

    int result;
    if (controller->version() == 2) {
        result = 0;
        if (!strcmp(controller->name(), CGROUPV2_CONTROLLER_NAME)) {
            // /sys/fs/cgroup is created by cgroup2 with specific selinux permissions,
            // try to create again in case the mount point is changed
            if (!Mkdir(controller->path(), 0, "", "")) {
                LOG(ERROR) << "Failed to create directory for " << controller->name() << " cgroup";
                return false;
            }

            result = mount("none", controller->path(), "cgroup2", MS_NODEV | MS_NOEXEC | MS_NOSUID,
                           nullptr);

            // selinux permissions change after mounting, so it's ok to change mode and owner now
            if (!ChangeDirModeAndOwner(controller->path(), descriptor.mode(), descriptor.uid(),
                                       descriptor.gid())) {
                LOG(ERROR) << "Failed to create directory for " << controller->name() << " cgroup";
                result = -1;
            }
        } else {
            if (!Mkdir(controller->path(), descriptor.mode(), descriptor.uid(), descriptor.gid())) {
                LOG(ERROR) << "Failed to create directory for " << controller->name() << " cgroup";
                return false;
            }

            if (controller->flags() & CGROUPRC_CONTROLLER_FLAG_NEEDS_ACTIVATION) {
                std::string str = std::string("+") + controller->name();
                std::string path = std::string(controller->path()) + "/cgroup.subtree_control";

                if (!base::WriteStringToFile(str, path)) {
                    LOG(ERROR) << "Failed to activate controller " << controller->name();
                    return false;
                }
            }
        }
    } else {
        // mkdir <path> [mode] [owner] [group]
        if (!Mkdir(controller->path(), descriptor.mode(), descriptor.uid(), descriptor.gid())) {
            LOG(ERROR) << "Failed to create directory for " << controller->name() << " cgroup";
            return false;
        }

        // Unfortunately historically cpuset controller was mounted using a mount command
        // different from all other controllers. This results in controller attributes not
        // to be prepended with controller name. For example this way instead of
        // /dev/cpuset/cpuset.cpus the attribute becomes /dev/cpuset/cpus which is what
        // the system currently expects.
        if (!strcmp(controller->name(), "cpuset")) {
            // mount cpuset none /dev/cpuset nodev noexec nosuid
            result = mount("none", controller->path(), controller->name(),
                           MS_NODEV | MS_NOEXEC | MS_NOSUID, nullptr);
        } else {
            // mount cgroup none <path> nodev noexec nosuid <controller>
            result = mount("none", controller->path(), "cgroup", MS_NODEV | MS_NOEXEC | MS_NOSUID,
                           controller->name());
        }
    }

    if (result < 0) {
        bool optional = controller->flags() & CGROUPRC_CONTROLLER_FLAG_OPTIONAL;

        if (optional && errno == EINVAL) {
            // Optional controllers are allowed to fail to mount if kernel does not support them
            LOG(INFO) << "Optional " << controller->name() << " cgroup controller is not mounted";
        } else {
            PLOG(ERROR) << "Failed to mount " << controller->name() << " cgroup";
            return false;
        }
    }

    return true;
}

#else

// Stubs for non-Android targets.
static bool SetupCgroup(const CgroupDescriptor&) {
    return false;
}

#endif

static bool WriteRcFile(const std::map<std::string, CgroupDescriptor>& descriptors) {
    unique_fd fd(TEMP_FAILURE_RETRY(open(CGROUPS_RC_PATH, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC,
                                         S_IRUSR | S_IRGRP | S_IROTH)));
    if (fd < 0) {
        PLOG(ERROR) << "open() failed for " << CGROUPS_RC_PATH;
        return false;
    }

    format::CgroupFile fl;
    fl.version_ = format::CgroupFile::FILE_CURR_VERSION;
    fl.controller_count_ = descriptors.size();
    int ret = TEMP_FAILURE_RETRY(write(fd, &fl, sizeof(fl)));
    if (ret < 0) {
        PLOG(ERROR) << "write() failed for " << CGROUPS_RC_PATH;
        return false;
    }

    for (const auto& [name, descriptor] : descriptors) {
        ret = TEMP_FAILURE_RETRY(
                write(fd, descriptor.controller(), sizeof(format::CgroupController)));
        if (ret < 0) {
            PLOG(ERROR) << "write() failed for " << CGROUPS_RC_PATH;
            return false;
        }
    }

    return true;
}

CgroupDescriptor::CgroupDescriptor(uint32_t version, const std::string& name,
                                   const std::string& path, mode_t mode, const std::string& uid,
                                   const std::string& gid, uint32_t flags = 0)
    : controller_(version, flags, name, path), mode_(mode), uid_(uid), gid_(gid) {}

void CgroupDescriptor::set_mounted(bool mounted) {
    uint32_t flags = controller_.flags();
    if (mounted) {
        flags |= CGROUPRC_CONTROLLER_FLAG_MOUNTED;
    } else {
        flags &= ~CGROUPRC_CONTROLLER_FLAG_MOUNTED;
    }
    controller_.set_flags(flags);
}

}  // namespace cgrouprc
}  // namespace android

bool CgroupSetup() {
    using namespace android::cgrouprc;

    std::map<std::string, CgroupDescriptor> descriptors;

    if (getpid() != 1) {
        LOG(ERROR) << "Cgroup setup can be done only by init process";
        return false;
    }

    // Make sure we do this only one time. No need for std::call_once because
    // init is a single-threaded process
    if (access(CGROUPS_RC_PATH, F_OK) == 0) {
        LOG(WARNING) << "Attempt to call SetupCgroups more than once";
        return true;
    }

    // load cgroups.json file
    if (!ReadDescriptors(&descriptors)) {
        LOG(ERROR) << "Failed to load cgroup description file";
        return false;
    }

    // setup cgroups
    for (auto& [name, descriptor] : descriptors) {
        if (SetupCgroup(descriptor)) {
            descriptor.set_mounted(true);
        } else {
            // issue a warning and proceed with the next cgroup
            LOG(WARNING) << "Failed to setup " << name << " cgroup";
        }
    }

    // mkdir <CGROUPS_RC_DIR> 0711 system system
    if (!Mkdir(android::base::Dirname(CGROUPS_RC_PATH), 0711, "system", "system")) {
        LOG(ERROR) << "Failed to create directory for " << CGROUPS_RC_PATH << " file";
        return false;
    }

    // Generate <CGROUPS_RC_FILE> file which can be directly mmapped into
    // process memory. This optimizes performance, memory usage
    // and limits infrormation shared with unprivileged processes
    // to the minimum subset of information from cgroups.json
    if (!WriteRcFile(descriptors)) {
        LOG(ERROR) << "Failed to write " << CGROUPS_RC_PATH << " file";
        return false;
    }

    // chmod 0644 <CGROUPS_RC_PATH>
    if (fchmodat(AT_FDCWD, CGROUPS_RC_PATH, 0644, AT_SYMLINK_NOFOLLOW) < 0) {
        PLOG(ERROR) << "fchmodat() failed";
        return false;
    }

    return true;
}
