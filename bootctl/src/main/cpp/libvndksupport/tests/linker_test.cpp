/*
 * Copyright (C) 2017 The Android Open Source Project
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
#include <gtest/gtest.h>

#include <android-base/strings.h>
#include <dirent.h>
#include <dlfcn.h>
#include <vndksupport/linker.h>
#include <string>

// Let's use libEGL_<chipset>.so as a SP-HAL in test
static std::string find_sphal_lib() {
    const char* path =
#if defined(__LP64__)
        "/vendor/lib64/egl";
#else
        "/vendor/lib/egl";
#endif
    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(path), closedir);

    dirent* dp;
    while ((dp = readdir(dir.get())) != nullptr) {
        std::string name = dp->d_name;
        if (android::base::StartsWith(name, "libEGL_")) {
            return std::string(path) + "/" + name;
        }
    }
    return "";
}

TEST(linker, load_existing_lib) {
    std::string name = find_sphal_lib();
    ASSERT_NE("", name);
    void* handle = android_load_sphal_library(name.c_str(), RTLD_NOW | RTLD_LOCAL);
    ASSERT_NE(nullptr, handle);
    android_unload_sphal_library(handle);
}

TEST(linker, load_nonexisting_lib) {
    void* handle = android_load_sphal_library("libNeverUseThisName.so", RTLD_NOW | RTLD_LOCAL);
    ASSERT_EQ(nullptr, handle);
}
