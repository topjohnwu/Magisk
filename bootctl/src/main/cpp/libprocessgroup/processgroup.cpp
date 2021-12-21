/*
 *  Copyright 2014 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "libprocessgroup"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <cutils/android_filesystem_config.h>
#include <processgroup/processgroup.h>
#include <task_profiles.h>

using android::base::GetBoolProperty;
using android::base::StartsWith;
using android::base::StringPrintf;
using android::base::WriteStringToFile;

using namespace std::chrono_literals;

#define PROCESSGROUP_CGROUP_PROCS_FILE "/cgroup.procs"

bool CgroupGetControllerPath(const std::string& cgroup_name, std::string* path) {
    auto controller = CgroupMap::GetInstance().FindController(cgroup_name);

    if (!controller.HasValue()) {
        return false;
    }

    if (path) {
        *path = controller.path();
    }

    return true;
}

bool CgroupGetAttributePath(const std::string& attr_name, std::string* path) {
    const TaskProfiles& tp = TaskProfiles::GetInstance();
    const ProfileAttribute* attr = tp.GetAttribute(attr_name);

    if (attr == nullptr) {
        return false;
    }

    if (path) {
        *path = StringPrintf("%s/%s", attr->controller()->path(), attr->file_name().c_str());
    }

    return true;
}

bool CgroupGetAttributePathForTask(const std::string& attr_name, int tid, std::string* path) {
    const TaskProfiles& tp = TaskProfiles::GetInstance();
    const ProfileAttribute* attr = tp.GetAttribute(attr_name);

    if (attr == nullptr) {
        return false;
    }

    if (!attr->GetPathForTask(tid, path)) {
        PLOG(ERROR) << "Failed to find cgroup for tid " << tid;
        return false;
    }

    return true;
}

bool UsePerAppMemcg() {
    bool low_ram_device = GetBoolProperty("ro.config.low_ram", false);
    return GetBoolProperty("ro.config.per_app_memcg", low_ram_device);
}

static bool isMemoryCgroupSupported() {
    static bool memcg_supported = CgroupMap::GetInstance().FindController("memory").IsUsable();

    return memcg_supported;
}

void DropTaskProfilesResourceCaching() {
    TaskProfiles::GetInstance().DropResourceCaching();
}

bool SetProcessProfiles(uid_t uid, pid_t pid, const std::vector<std::string>& profiles) {
    return TaskProfiles::GetInstance().SetProcessProfiles(uid, pid, profiles);
}

bool SetTaskProfiles(int tid, const std::vector<std::string>& profiles, bool use_fd_cache) {
    return TaskProfiles::GetInstance().SetTaskProfiles(tid, profiles, use_fd_cache);
}

static std::string ConvertUidToPath(const char* cgroup, uid_t uid) {
    return StringPrintf("%s/uid_%d", cgroup, uid);
}

static std::string ConvertUidPidToPath(const char* cgroup, uid_t uid, int pid) {
    return StringPrintf("%s/uid_%d/pid_%d", cgroup, uid, pid);
}

static int RemoveProcessGroup(const char* cgroup, uid_t uid, int pid, unsigned int retries) {
    int ret = 0;
    auto uid_pid_path = ConvertUidPidToPath(cgroup, uid, pid);
    auto uid_path = ConvertUidToPath(cgroup, uid);

    if (retries == 0) {
        retries = 1;
    }

    while (retries--) {
        ret = rmdir(uid_pid_path.c_str());
        if (!ret || errno != EBUSY) break;
        std::this_thread::sleep_for(5ms);
    }

    return ret;
}

static bool RemoveUidProcessGroups(const std::string& uid_path) {
    std::unique_ptr<DIR, decltype(&closedir)> uid(opendir(uid_path.c_str()), closedir);
    bool empty = true;
    if (uid != NULL) {
        dirent* dir;
        while ((dir = readdir(uid.get())) != nullptr) {
            if (dir->d_type != DT_DIR) {
                continue;
            }

            if (!StartsWith(dir->d_name, "pid_")) {
                continue;
            }

            auto path = StringPrintf("%s/%s", uid_path.c_str(), dir->d_name);
            LOG(VERBOSE) << "Removing " << path;
            if (rmdir(path.c_str()) == -1) {
                if (errno != EBUSY) {
                    PLOG(WARNING) << "Failed to remove " << path;
                }
                empty = false;
            }
        }
    }
    return empty;
}

void removeAllProcessGroups() {
    LOG(VERBOSE) << "removeAllProcessGroups()";

    std::vector<std::string> cgroups;
    std::string path;

    if (CgroupGetControllerPath(CGROUPV2_CONTROLLER_NAME, &path)) {
        cgroups.push_back(path);
    }
    if (CgroupGetControllerPath("memory", &path)) {
        cgroups.push_back(path + "/apps");
    }

    for (std::string cgroup_root_path : cgroups) {
        std::unique_ptr<DIR, decltype(&closedir)> root(opendir(cgroup_root_path.c_str()), closedir);
        if (root == NULL) {
            PLOG(ERROR) << "Failed to open " << cgroup_root_path;
        } else {
            dirent* dir;
            while ((dir = readdir(root.get())) != nullptr) {
                if (dir->d_type != DT_DIR) {
                    continue;
                }

                if (!StartsWith(dir->d_name, "uid_")) {
                    continue;
                }

                auto path = StringPrintf("%s/%s", cgroup_root_path.c_str(), dir->d_name);
                if (!RemoveUidProcessGroups(path)) {
                    LOG(VERBOSE) << "Skip removing " << path;
                    continue;
                }
                LOG(VERBOSE) << "Removing " << path;
                if (rmdir(path.c_str()) == -1 && errno != EBUSY) {
                    PLOG(WARNING) << "Failed to remove " << path;
                }
            }
        }
    }
}

/**
 * Process groups are primarily created by the Zygote, meaning that uid/pid groups are created by
 * the user root. Ownership for the newly created cgroup and all of its files must thus be
 * transferred for the user/group passed as uid/gid before system_server can properly access them.
 */
static bool MkdirAndChown(const std::string& path, mode_t mode, uid_t uid, gid_t gid) {
    if (mkdir(path.c_str(), mode) == -1) {
        if (errno == EEXIST) {
            // Directory already exists and permissions have been set at the time it was created
            return true;
        }
        return false;
    }

    auto dir = std::unique_ptr<DIR, decltype(&closedir)>(opendir(path.c_str()), closedir);

    if (dir == NULL) {
        PLOG(ERROR) << "opendir failed for " << path;
        goto err;
    }

    struct dirent* dir_entry;
    while ((dir_entry = readdir(dir.get()))) {
        if (!strcmp("..", dir_entry->d_name)) {
            continue;
        }

        std::string file_path = path + "/" + dir_entry->d_name;

        if (lchown(file_path.c_str(), uid, gid) < 0) {
            PLOG(ERROR) << "lchown failed for " << file_path;
            goto err;
        }

        if (fchmodat(AT_FDCWD, file_path.c_str(), mode, AT_SYMLINK_NOFOLLOW) != 0) {
            PLOG(ERROR) << "fchmodat failed for " << file_path;
            goto err;
        }
    }

    return true;
err:
    int saved_errno = errno;
    rmdir(path.c_str());
    errno = saved_errno;

    return false;
}

// Returns number of processes killed on success
// Returns 0 if there are no processes in the process cgroup left to kill
// Returns -1 on error
static int DoKillProcessGroupOnce(const char* cgroup, uid_t uid, int initialPid, int signal) {
    auto path = ConvertUidPidToPath(cgroup, uid, initialPid) + PROCESSGROUP_CGROUP_PROCS_FILE;
    std::unique_ptr<FILE, decltype(&fclose)> fd(fopen(path.c_str(), "re"), fclose);
    if (!fd) {
        if (errno == ENOENT) {
            // This happens when process is already dead
            return 0;
        }
        PLOG(WARNING) << "Failed to open process cgroup uid " << uid << " pid " << initialPid;
        return -1;
    }

    // We separate all of the pids in the cgroup into those pids that are also the leaders of
    // process groups (stored in the pgids set) and those that are not (stored in the pids set).
    std::set<pid_t> pgids;
    pgids.emplace(initialPid);
    std::set<pid_t> pids;

    pid_t pid;
    int processes = 0;
    while (fscanf(fd.get(), "%d\n", &pid) == 1 && pid >= 0) {
        processes++;
        if (pid == 0) {
            // Should never happen...  but if it does, trying to kill this
            // will boomerang right back and kill us!  Let's not let that happen.
            LOG(WARNING) << "Yikes, we've been told to kill pid 0!  How about we don't do that?";
            continue;
        }
        pid_t pgid = getpgid(pid);
        if (pgid == -1) PLOG(ERROR) << "getpgid(" << pid << ") failed";
        if (pgid == pid) {
            pgids.emplace(pid);
        } else {
            pids.emplace(pid);
        }
    }

    // Erase all pids that will be killed when we kill the process groups.
    for (auto it = pids.begin(); it != pids.end();) {
        pid_t pgid = getpgid(*it);
        if (pgids.count(pgid) == 1) {
            it = pids.erase(it);
        } else {
            ++it;
        }
    }

    // Kill all process groups.
    for (const auto pgid : pgids) {
        LOG(VERBOSE) << "Killing process group " << -pgid << " in uid " << uid
                     << " as part of process cgroup " << initialPid;

        if (kill(-pgid, signal) == -1 && errno != ESRCH) {
            PLOG(WARNING) << "kill(" << -pgid << ", " << signal << ") failed";
        }
    }

    // Kill remaining pids.
    for (const auto pid : pids) {
        LOG(VERBOSE) << "Killing pid " << pid << " in uid " << uid << " as part of process cgroup "
                     << initialPid;

        if (kill(pid, signal) == -1 && errno != ESRCH) {
            PLOG(WARNING) << "kill(" << pid << ", " << signal << ") failed";
        }
    }

    return feof(fd.get()) ? processes : -1;
}

static int KillProcessGroup(uid_t uid, int initialPid, int signal, int retries,
                            int* max_processes) {
    std::string hierarchy_root_path;
    CgroupGetControllerPath(CGROUPV2_CONTROLLER_NAME, &hierarchy_root_path);
    const char* cgroup = hierarchy_root_path.c_str();

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    if (max_processes != nullptr) {
        *max_processes = 0;
    }

    int retry = retries;
    int processes;
    while ((processes = DoKillProcessGroupOnce(cgroup, uid, initialPid, signal)) > 0) {
        if (max_processes != nullptr && processes > *max_processes) {
            *max_processes = processes;
        }
        LOG(VERBOSE) << "Killed " << processes << " processes for processgroup " << initialPid;
        if (retry > 0) {
            std::this_thread::sleep_for(5ms);
            --retry;
        } else {
            break;
        }
    }

    if (processes < 0) {
        PLOG(ERROR) << "Error encountered killing process cgroup uid " << uid << " pid "
                    << initialPid;
        return -1;
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // We only calculate the number of 'processes' when killing the processes.
    // In the retries == 0 case, we only kill the processes once and therefore
    // will not have waited then recalculated how many processes are remaining
    // after the first signals have been sent.
    // Logging anything regarding the number of 'processes' here does not make sense.

    if (processes == 0) {
        if (retries > 0) {
            LOG(INFO) << "Successfully killed process cgroup uid " << uid << " pid " << initialPid
                      << " in " << static_cast<int>(ms) << "ms";
        }

        int err = RemoveProcessGroup(cgroup, uid, initialPid, retries);

        if (isMemoryCgroupSupported() && UsePerAppMemcg()) {
            std::string memory_path;
            CgroupGetControllerPath("memory", &memory_path);
            memory_path += "/apps";
            if (RemoveProcessGroup(memory_path.c_str(), uid, initialPid, retries)) return -1;
        }

        return err;
    } else {
        if (retries > 0) {
            LOG(ERROR) << "Failed to kill process cgroup uid " << uid << " pid " << initialPid
                       << " in " << static_cast<int>(ms) << "ms, " << processes
                       << " processes remain";
        }
        return -1;
    }
}

int killProcessGroup(uid_t uid, int initialPid, int signal, int* max_processes) {
    return KillProcessGroup(uid, initialPid, signal, 40 /*retries*/, max_processes);
}

int killProcessGroupOnce(uid_t uid, int initialPid, int signal, int* max_processes) {
    return KillProcessGroup(uid, initialPid, signal, 0 /*retries*/, max_processes);
}

static int createProcessGroupInternal(uid_t uid, int initialPid, std::string cgroup) {
    auto uid_path = ConvertUidToPath(cgroup.c_str(), uid);

    struct stat cgroup_stat;
    mode_t cgroup_mode = 0750;
    gid_t cgroup_uid = AID_SYSTEM;
    uid_t cgroup_gid = AID_SYSTEM;

    if (stat(cgroup.c_str(), &cgroup_stat) == 1) {
        PLOG(ERROR) << "Failed to get stats for " << cgroup;
    } else {
        cgroup_mode = cgroup_stat.st_mode;
        cgroup_uid = cgroup_stat.st_uid;
        cgroup_gid = cgroup_stat.st_gid;
    }

    if (!MkdirAndChown(uid_path, cgroup_mode, cgroup_uid, cgroup_gid)) {
        PLOG(ERROR) << "Failed to make and chown " << uid_path;
        return -errno;
    }

    auto uid_pid_path = ConvertUidPidToPath(cgroup.c_str(), uid, initialPid);

    if (!MkdirAndChown(uid_pid_path, cgroup_mode, cgroup_uid, cgroup_gid)) {
        PLOG(ERROR) << "Failed to make and chown " << uid_pid_path;
        return -errno;
    }

    auto uid_pid_procs_file = uid_pid_path + PROCESSGROUP_CGROUP_PROCS_FILE;

    int ret = 0;
    if (!WriteStringToFile(std::to_string(initialPid), uid_pid_procs_file)) {
        ret = -errno;
        PLOG(ERROR) << "Failed to write '" << initialPid << "' to " << uid_pid_procs_file;
    }

    return ret;
}

int createProcessGroup(uid_t uid, int initialPid, bool memControl) {
    std::string cgroup;

    if (memControl && !UsePerAppMemcg()) {
        PLOG(ERROR) << "service memory controls are used without per-process memory cgroup support";
        return -EINVAL;
    }

    if (isMemoryCgroupSupported() && UsePerAppMemcg()) {
        CgroupGetControllerPath("memory", &cgroup);
        cgroup += "/apps";
        int ret = createProcessGroupInternal(uid, initialPid, cgroup);
        if (ret != 0) {
            return ret;
        }
    }

    CgroupGetControllerPath(CGROUPV2_CONTROLLER_NAME, &cgroup);
    return createProcessGroupInternal(uid, initialPid, cgroup);
}

static bool SetProcessGroupValue(int tid, const std::string& attr_name, int64_t value) {
    if (!isMemoryCgroupSupported()) {
        PLOG(ERROR) << "Memcg is not mounted.";
        return false;
    }

    std::string path;
    if (!CgroupGetAttributePathForTask(attr_name, tid, &path)) {
        PLOG(ERROR) << "Failed to find attribute '" << attr_name << "'";
        return false;
    }

    if (!WriteStringToFile(std::to_string(value), path)) {
        PLOG(ERROR) << "Failed to write '" << value << "' to " << path;
        return false;
    }
    return true;
}

bool setProcessGroupSwappiness(uid_t, int pid, int swappiness) {
    return SetProcessGroupValue(pid, "MemSwappiness", swappiness);
}

bool setProcessGroupSoftLimit(uid_t, int pid, int64_t soft_limit_in_bytes) {
    return SetProcessGroupValue(pid, "MemSoftLimit", soft_limit_in_bytes);
}

bool setProcessGroupLimit(uid_t, int pid, int64_t limit_in_bytes) {
    return SetProcessGroupValue(pid, "MemLimit", limit_in_bytes);
}

bool getAttributePathForTask(const std::string& attr_name, int tid, std::string* path) {
    return CgroupGetAttributePathForTask(attr_name, tid, path);
}
