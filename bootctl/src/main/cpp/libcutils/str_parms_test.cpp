/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include <cutils/str_parms.h>
#include <gtest/gtest.h>

static void test_str_parms_str(const char* str, const char* expected) {
    str_parms* str_parms = str_parms_create_str(str);
    str_parms_add_str(str_parms, "dude", "woah");
    str_parms_add_str(str_parms, "dude", "woah");
    str_parms_del(str_parms, "dude");
    str_parms_dump(str_parms);
    char* out_str = str_parms_to_str(str_parms);
    str_parms_destroy(str_parms);
    ASSERT_STREQ(expected, out_str) << str;
    free(out_str);
}

TEST(str_parms, smoke) {
    test_str_parms_str("", "");
    test_str_parms_str(";", "");
    test_str_parms_str("=", "");
    test_str_parms_str("=;", "");
    test_str_parms_str("=bar", "");
    test_str_parms_str("=bar;", "");
    test_str_parms_str("foo=", "foo=");
    test_str_parms_str("foo=;", "foo=");
    test_str_parms_str("foo=bar", "foo=bar");
    test_str_parms_str("foo=bar;", "foo=bar");
    test_str_parms_str("foo=bar;baz", "foo=bar;baz=");
    test_str_parms_str("foo=bar;baz=", "foo=bar;baz=");
    test_str_parms_str("foo=bar;baz=bat", "foo=bar;baz=bat");
    test_str_parms_str("foo=bar;baz=bat;", "foo=bar;baz=bat");
    test_str_parms_str("foo=bar1;baz=bat;foo=bar2", "foo=bar2;baz=bat");
}

TEST(str_parms, put_ENOMEM) {
    // hashmapPut reports errors by setting errno to ENOMEM.
    // Test that we're not confused by running in an environment where this is already true.
    errno = ENOMEM;
    test_str_parms_str("foo=bar;baz=", "foo=bar;baz=");
    ASSERT_EQ(ENOMEM, errno);
    test_str_parms_str("foo=bar;baz=", "foo=bar;baz=");
}
