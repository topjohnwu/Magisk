//
// Copyright (C) 2017 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <android-base/logging.h>
#include <libminijail.h>

#include <hwminijail/HardwareMinijail.h>

namespace android {
namespace hardware {

void SetupMinijail(const std::string& seccomp_policy_path) {
    if (access(seccomp_policy_path.c_str(), R_OK) == -1) {
        LOG(WARNING) << "Could not find seccomp policy file at: " << seccomp_policy_path;
        return;
    }

    struct minijail* jail = minijail_new();
    if (jail == nullptr) {
        LOG(FATAL) << "Failed to create minijail.";
    }

    minijail_no_new_privs(jail);
    minijail_log_seccomp_filter_failures(jail);
    minijail_use_seccomp_filter(jail);
    minijail_parse_seccomp_filters(jail, seccomp_policy_path.c_str());
    minijail_enter(jail);
    minijail_destroy(jail);
}

}  // namespace hardware
}  // namespace android
