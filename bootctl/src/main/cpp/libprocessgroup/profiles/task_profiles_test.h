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

#include "task_profiles.pb.h"

namespace android {
namespace profiles {

class TaskProfilesTest : public jsonpb::JsonSchemaTest {
  public:
    void SetUp() override {
        JsonSchemaTest::SetUp();
        task_profiles_ = static_cast<TaskProfiles*>(message());
    }
    TaskProfiles* task_profiles_;
};

TEST_P(TaskProfilesTest, AttributeRequiredFields) {
    for (int i = 0; i < task_profiles_->attributes_size(); ++i) {
        auto&& attribute = task_profiles_->attributes(i);
        EXPECT_FALSE(attribute.name().empty())
                << "No name for attribute #" << i << " in " << file_path_;
        EXPECT_FALSE(attribute.controller().empty())
                << "No controller for attribute #" << i << " in " << file_path_;
        EXPECT_FALSE(attribute.file().empty())
                << "No file for attribute #" << i << " in " << file_path_;
    }
}

TEST_P(TaskProfilesTest, ProfileRequiredFields) {
    for (int profile_idx = 0; profile_idx < task_profiles_->profiles_size(); ++profile_idx) {
        auto&& profile = task_profiles_->profiles(profile_idx);
        EXPECT_FALSE(profile.name().empty())
                << "No name for profile #" << profile_idx << " in " << file_path_;
        for (int action_idx = 0; action_idx < profile.actions_size(); ++action_idx) {
            auto&& action = profile.actions(action_idx);
            EXPECT_FALSE(action.name().empty())
                    << "No name for profiles[" << profile_idx << "].actions[" << action_idx
                    << "] in " << file_path_;
        }
    }
}

}  // namespace profiles
}  // namespace android
