/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#include <string>

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <cutils/android_get_control_file.h>
#include <gtest/gtest.h>

TEST(FilesTest, android_get_control_file) {
    TemporaryFile tf;
    ASSERT_GE(tf.fd, 0);

    std::string key(ANDROID_FILE_ENV_PREFIX);
    key += tf.path;

    std::for_each(key.begin(), key.end(), [] (char& c) { c = isalnum(c) ? c : '_'; });

    EXPECT_EQ(unsetenv(key.c_str()), 0);
    EXPECT_EQ(android_get_control_file(tf.path), -1);

    EXPECT_EQ(setenv(key.c_str(), android::base::StringPrintf("%d", tf.fd).c_str(), true), 0);

    EXPECT_EQ(android_get_control_file(tf.path), tf.fd);
    close(tf.fd);
    EXPECT_EQ(android_get_control_file(tf.path), -1);
    EXPECT_EQ(unsetenv(key.c_str()), 0);
    EXPECT_EQ(android_get_control_file(tf.path), -1);
}
