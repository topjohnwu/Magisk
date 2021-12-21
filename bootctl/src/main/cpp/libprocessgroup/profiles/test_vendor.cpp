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

using ::android::base::GetExecutableDirectory;
using namespace ::android::jsonpb;

namespace android {
namespace profiles {

static constexpr const char* kVendorCgroups = "/vendor/etc/cgroups.json";
static constexpr const char* kVendorTaskProfiles = "/vendor/etc/task_profiles.json";

template <typename T>
class TestConfig : public JsonSchemaTestConfig {
  public:
    TestConfig(const std::string& path) : file_path_(path){};
    std::unique_ptr<google::protobuf::Message> CreateMessage() const override {
        return std::make_unique<T>();
    }
    std::string file_path() const override { return file_path_; }
    bool optional() const override {
        // Ignore when vendor JSON files are missing.
        return true;
    }

  private:
    std::string file_path_;
};

template <typename T>
JsonSchemaTestConfigFactory MakeTestParam(const std::string& path) {
    return [path]() { return std::make_unique<TestConfig<T>>(path); };
}

INSTANTIATE_TEST_SUITE_P(VendorCgroups, JsonSchemaTest,
                         ::testing::Values(MakeTestParam<Cgroups>(kVendorCgroups)));
INSTANTIATE_TEST_SUITE_P(VendorCgroups, CgroupsTest,
                         ::testing::Values(MakeTestParam<Cgroups>(kVendorCgroups)));

INSTANTIATE_TEST_SUITE_P(VendorTaskProfiles, JsonSchemaTest,
                         ::testing::Values(MakeTestParam<TaskProfiles>(kVendorTaskProfiles)));
INSTANTIATE_TEST_SUITE_P(VendorTaskProfiles, TaskProfilesTest,
                         ::testing::Values(MakeTestParam<TaskProfiles>(kVendorTaskProfiles)));

}  // namespace profiles
}  // namespace android

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
