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

#include <android-base/file.h>
#include <gtest/gtest.h>
#include <jsonpb/json_schema_test.h>

#include "cgroups_test.h"
#include "task_profiles_test.h"

using namespace ::android::jsonpb;
using ::android::base::GetExecutableDirectory;

namespace android {
namespace profiles {

template <typename T>
JsonSchemaTestConfigFactory MakeTestParam(const std::string& path) {
    return jsonpb::MakeTestParam<T>(GetExecutableDirectory() + path);
}

// Test suite instantiations
INSTANTIATE_TEST_SUITE_P(Cgroups, JsonSchemaTest,
                         ::testing::Values(MakeTestParam<Cgroups>("/cgroups.json"),
                                           MakeTestParam<Cgroups>("/cgroups.recovery.json"),
                                           MakeTestParam<TaskProfiles>("/task_profiles.json")));
INSTANTIATE_TEST_SUITE_P(Cgroups, CgroupsTest,
                         ::testing::Values(MakeTestParam<Cgroups>("/cgroups.json"),
                                           MakeTestParam<Cgroups>("/cgroups.recovery.json")));
INSTANTIATE_TEST_SUITE_P(TaskProfiles, TaskProfilesTest,
                         ::testing::Values(MakeTestParam<TaskProfiles>("/task_profiles.json")));

}  // namespace profiles
}  // namespace android

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
