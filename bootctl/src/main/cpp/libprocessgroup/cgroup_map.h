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

#include <sys/cdefs.h>
#include <sys/types.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <android/cgrouprc.h>

// Convenient wrapper of an ACgroupController pointer.
class CgroupController {
  public:
    // Does not own controller
    explicit CgroupController(const ACgroupController* controller)
        : controller_(controller), state_(UNKNOWN) {}

    uint32_t version() const;
    const char* name() const;
    const char* path() const;

    bool HasValue() const;
    bool IsUsable();

    std::string GetTasksFilePath(const std::string& path) const;
    std::string GetProcsFilePath(const std::string& path, uid_t uid, pid_t pid) const;
    bool GetTaskGroup(int tid, std::string* group) const;
  private:
    enum ControllerState {
        UNKNOWN = 0,
        USABLE = 1,
        MISSING = 2,
    };

    const ACgroupController* controller_ = nullptr;
    ControllerState state_;
};

class CgroupMap {
  public:
    // Selinux policy ensures only init process can successfully use this function
    static bool SetupCgroups();

    static CgroupMap& GetInstance();
    CgroupController FindController(const std::string& name) const;

  private:
    bool loaded_ = false;
    CgroupMap();
    bool LoadRcFile();
    void Print() const;
};
