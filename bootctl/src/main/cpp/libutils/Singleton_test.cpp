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

#define LOG_TAG "Singleton_test"

#include <dlfcn.h>

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <utils/Singleton.h>

#include <gtest/gtest.h>

#include "Singleton_test.h"

namespace android {

TEST(SingletonTest, bug35674422) {
    std::string path = android::base::GetExecutableDirectory();
    // libutils_test_singleton1.so contains the ANDROID_SINGLETON_STATIC_INSTANCE
    // definition of SingletonTestData, load it first.
    std::string lib = android::base::StringPrintf("%s/libutils_test_singleton1.so", path.c_str());
    void* handle1 = dlopen(lib.c_str(), RTLD_NOW);
    ASSERT_TRUE(handle1 != nullptr) << dlerror();

    // libutils_test_singleton2.so references SingletonTestData but should not
    // have a definition
    lib = android::base::StringPrintf("%s/libutils_test_singleton2.so", path.c_str());
    void* handle2 = dlopen(lib.c_str(), RTLD_NOW);
    ASSERT_TRUE(handle2 != nullptr) << dlerror();

    using has_fn_t = decltype(&singletonHasInstance);
    using get_fn_t = decltype(&singletonGetInstanceContents);
    using set_fn_t = decltype(&singletonSetInstanceContents);

    has_fn_t has1 = reinterpret_cast<has_fn_t>(dlsym(handle1, "singletonHasInstance"));
    ASSERT_TRUE(has1 != nullptr) << dlerror();
    has_fn_t has2 = reinterpret_cast<has_fn_t>(dlsym(handle2, "singletonHasInstance"));
    ASSERT_TRUE(has2 != nullptr) << dlerror();
    get_fn_t get1 = reinterpret_cast<get_fn_t>(dlsym(handle1, "singletonGetInstanceContents"));
    ASSERT_TRUE(get1 != nullptr) << dlerror();
    get_fn_t get2 = reinterpret_cast<get_fn_t>(dlsym(handle2, "singletonGetInstanceContents"));
    ASSERT_TRUE(get2 != nullptr) << dlerror();
    set_fn_t set1 = reinterpret_cast<set_fn_t>(dlsym(handle2, "singletonSetInstanceContents"));
    ASSERT_TRUE(set1 != nullptr) << dlerror();

    EXPECT_FALSE(has1());
    EXPECT_FALSE(has2());
    set1(12345678U);
    EXPECT_TRUE(has1());
    EXPECT_TRUE(has2());
    EXPECT_EQ(12345678U, get1());
    EXPECT_EQ(12345678U, get2());
}

}
