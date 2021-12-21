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

#include <string>

#include <gmock/gmock.h>
#include <jsonpb/json_schema_test.h>

#include "cgroups.pb.h"

using ::testing::MatchesRegex;

namespace android {
namespace profiles {

class CgroupsTest : public jsonpb::JsonSchemaTest {
  public:
    void SetUp() override {
        JsonSchemaTest::SetUp();
        cgroups_ = static_cast<Cgroups*>(message());
    }
    Cgroups* cgroups_;
};

TEST_P(CgroupsTest, CgroupRequiredFields) {
    for (int i = 0; i < cgroups_->cgroups_size(); ++i) {
        auto&& cgroup = cgroups_->cgroups(i);
        EXPECT_FALSE(cgroup.controller().empty())
                << "No controller name for cgroup #" << i << " in " << file_path_;
        EXPECT_FALSE(cgroup.path().empty()) << "No path for cgroup #" << i << " in " << file_path_;
    }
}

TEST_P(CgroupsTest, Cgroup2RequiredFields) {
    if (cgroups_->has_cgroups2()) {
        EXPECT_FALSE(cgroups_->cgroups2().path().empty())
                << "No path for cgroup2 in " << file_path_;
    }
}

// "Mode" field must be in the format of "0xxx".
static inline constexpr const char* REGEX_MODE = "(0[0-7]{3})?";
TEST_P(CgroupsTest, CgroupMode) {
    for (int i = 0; i < cgroups_->cgroups_size(); ++i) {
        EXPECT_THAT(cgroups_->cgroups(i).mode(), MatchesRegex(REGEX_MODE))
                << "For cgroup controller #" << i << " in " << file_path_;
    }
}

TEST_P(CgroupsTest, Cgroup2Mode) {
    EXPECT_THAT(cgroups_->cgroups2().mode(), MatchesRegex(REGEX_MODE))
            << "For cgroups2 in " << file_path_;
}

}  // namespace profiles
}  // namespace android
