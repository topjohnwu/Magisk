/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "android-base/strings.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <set>
#include <unordered_set>

TEST(strings, split_empty) {
  std::vector<std::string> parts = android::base::Split("", ",");
  ASSERT_EQ(1U, parts.size());
  ASSERT_EQ("", parts[0]);
}

TEST(strings, split_single) {
  std::vector<std::string> parts = android::base::Split("foo", ",");
  ASSERT_EQ(1U, parts.size());
  ASSERT_EQ("foo", parts[0]);
}

TEST(strings, split_simple) {
  std::vector<std::string> parts = android::base::Split("foo,bar,baz", ",");
  ASSERT_EQ(3U, parts.size());
  ASSERT_EQ("foo", parts[0]);
  ASSERT_EQ("bar", parts[1]);
  ASSERT_EQ("baz", parts[2]);
}

TEST(strings, split_with_empty_part) {
  std::vector<std::string> parts = android::base::Split("foo,,bar", ",");
  ASSERT_EQ(3U, parts.size());
  ASSERT_EQ("foo", parts[0]);
  ASSERT_EQ("", parts[1]);
  ASSERT_EQ("bar", parts[2]);
}

TEST(strings, split_with_trailing_empty_part) {
  std::vector<std::string> parts = android::base::Split("foo,bar,", ",");
  ASSERT_EQ(3U, parts.size());
  ASSERT_EQ("foo", parts[0]);
  ASSERT_EQ("bar", parts[1]);
  ASSERT_EQ("", parts[2]);
}

TEST(strings, split_null_char) {
  std::vector<std::string> parts =
      android::base::Split(std::string("foo\0bar", 7), std::string("\0", 1));
  ASSERT_EQ(2U, parts.size());
  ASSERT_EQ("foo", parts[0]);
  ASSERT_EQ("bar", parts[1]);
}

TEST(strings, split_any) {
  std::vector<std::string> parts = android::base::Split("foo:bar,baz", ",:");
  ASSERT_EQ(3U, parts.size());
  ASSERT_EQ("foo", parts[0]);
  ASSERT_EQ("bar", parts[1]);
  ASSERT_EQ("baz", parts[2]);
}

TEST(strings, split_any_with_empty_part) {
  std::vector<std::string> parts = android::base::Split("foo:,bar", ",:");
  ASSERT_EQ(3U, parts.size());
  ASSERT_EQ("foo", parts[0]);
  ASSERT_EQ("", parts[1]);
  ASSERT_EQ("bar", parts[2]);
}

TEST(strings, trim_empty) {
  ASSERT_EQ("", android::base::Trim(""));
}

TEST(strings, trim_already_trimmed) {
  ASSERT_EQ("foo", android::base::Trim("foo"));
}

TEST(strings, trim_left) {
  ASSERT_EQ("foo", android::base::Trim(" foo"));
}

TEST(strings, trim_right) {
  ASSERT_EQ("foo", android::base::Trim("foo "));
}

TEST(strings, trim_both) {
  ASSERT_EQ("foo", android::base::Trim(" foo "));
}

TEST(strings, trim_no_trim_middle) {
  ASSERT_EQ("foo bar", android::base::Trim("foo bar"));
}

TEST(strings, trim_other_whitespace) {
  ASSERT_EQ("foo", android::base::Trim("\v\tfoo\n\f"));
}

TEST(strings, join_nothing) {
  std::vector<std::string> list = {};
  ASSERT_EQ("", android::base::Join(list, ','));
}

TEST(strings, join_single) {
  std::vector<std::string> list = {"foo"};
  ASSERT_EQ("foo", android::base::Join(list, ','));
}

TEST(strings, join_simple) {
  std::vector<std::string> list = {"foo", "bar", "baz"};
  ASSERT_EQ("foo,bar,baz", android::base::Join(list, ','));
}

TEST(strings, join_separator_in_vector) {
  std::vector<std::string> list = {",", ","};
  ASSERT_EQ(",,,", android::base::Join(list, ','));
}

TEST(strings, join_simple_ints) {
  std::set<int> list = {1, 2, 3};
  ASSERT_EQ("1,2,3", android::base::Join(list, ','));
}

TEST(strings, join_unordered_set) {
  std::unordered_set<int> list = {1, 2};
  ASSERT_TRUE("1,2" == android::base::Join(list, ',') ||
              "2,1" == android::base::Join(list, ','));
}

TEST(strings, StartsWith_empty) {
  ASSERT_FALSE(android::base::StartsWith("", "foo"));
  ASSERT_TRUE(android::base::StartsWith("", ""));
}

TEST(strings, StartsWithIgnoreCase_empty) {
  ASSERT_FALSE(android::base::StartsWithIgnoreCase("", "foo"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("", ""));
}

TEST(strings, StartsWith_simple) {
  ASSERT_TRUE(android::base::StartsWith("foo", ""));
  ASSERT_TRUE(android::base::StartsWith("foo", "f"));
  ASSERT_TRUE(android::base::StartsWith("foo", "fo"));
  ASSERT_TRUE(android::base::StartsWith("foo", "foo"));
}

TEST(strings, StartsWithIgnoreCase_simple) {
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", ""));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "f"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "F"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "fo"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "fO"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "Fo"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "FO"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "foo"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "foO"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "fOo"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "fOO"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "Foo"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "FoO"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "FOo"));
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("foo", "FOO"));
}

TEST(strings, StartsWith_prefix_too_long) {
  ASSERT_FALSE(android::base::StartsWith("foo", "foobar"));
}

TEST(strings, StartsWithIgnoreCase_prefix_too_long) {
  ASSERT_FALSE(android::base::StartsWithIgnoreCase("foo", "foobar"));
  ASSERT_FALSE(android::base::StartsWithIgnoreCase("foo", "FOOBAR"));
}

TEST(strings, StartsWith_contains_prefix) {
  ASSERT_FALSE(android::base::StartsWith("foobar", "oba"));
  ASSERT_FALSE(android::base::StartsWith("foobar", "bar"));
}

TEST(strings, StartsWithIgnoreCase_contains_prefix) {
  ASSERT_FALSE(android::base::StartsWithIgnoreCase("foobar", "oba"));
  ASSERT_FALSE(android::base::StartsWithIgnoreCase("foobar", "OBA"));
  ASSERT_FALSE(android::base::StartsWithIgnoreCase("foobar", "bar"));
  ASSERT_FALSE(android::base::StartsWithIgnoreCase("foobar", "BAR"));
}

TEST(strings, StartsWith_char) {
  ASSERT_FALSE(android::base::StartsWith("", 'f'));
  ASSERT_TRUE(android::base::StartsWith("foo", 'f'));
  ASSERT_FALSE(android::base::StartsWith("foo", 'o'));
}

TEST(strings, EndsWith_empty) {
  ASSERT_FALSE(android::base::EndsWith("", "foo"));
  ASSERT_TRUE(android::base::EndsWith("", ""));
}

TEST(strings, EndsWithIgnoreCase_empty) {
  ASSERT_FALSE(android::base::EndsWithIgnoreCase("", "foo"));
  ASSERT_FALSE(android::base::EndsWithIgnoreCase("", "FOO"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("", ""));
}

TEST(strings, EndsWith_simple) {
  ASSERT_TRUE(android::base::EndsWith("foo", ""));
  ASSERT_TRUE(android::base::EndsWith("foo", "o"));
  ASSERT_TRUE(android::base::EndsWith("foo", "oo"));
  ASSERT_TRUE(android::base::EndsWith("foo", "foo"));
}

TEST(strings, EndsWithIgnoreCase_simple) {
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", ""));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "o"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "O"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "oo"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "oO"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "Oo"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "OO"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "foo"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "foO"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "fOo"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "fOO"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "Foo"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "FoO"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "FOo"));
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("foo", "FOO"));
}

TEST(strings, EndsWith_prefix_too_long) {
  ASSERT_FALSE(android::base::EndsWith("foo", "foobar"));
}

TEST(strings, EndsWithIgnoreCase_prefix_too_long) {
  ASSERT_FALSE(android::base::EndsWithIgnoreCase("foo", "foobar"));
  ASSERT_FALSE(android::base::EndsWithIgnoreCase("foo", "FOOBAR"));
}

TEST(strings, EndsWith_contains_prefix) {
  ASSERT_FALSE(android::base::EndsWith("foobar", "oba"));
  ASSERT_FALSE(android::base::EndsWith("foobar", "foo"));
}

TEST(strings, EndsWithIgnoreCase_contains_prefix) {
  ASSERT_FALSE(android::base::EndsWithIgnoreCase("foobar", "OBA"));
  ASSERT_FALSE(android::base::EndsWithIgnoreCase("foobar", "FOO"));
}

TEST(strings, StartsWith_std_string) {
  ASSERT_TRUE(android::base::StartsWith("hello", std::string{"hell"}));
  ASSERT_FALSE(android::base::StartsWith("goodbye", std::string{"hell"}));
}

TEST(strings, StartsWithIgnoreCase_std_string) {
  ASSERT_TRUE(android::base::StartsWithIgnoreCase("HeLlO", std::string{"hell"}));
  ASSERT_FALSE(android::base::StartsWithIgnoreCase("GoOdByE", std::string{"hell"}));
}

TEST(strings, EndsWith_std_string) {
  ASSERT_TRUE(android::base::EndsWith("hello", std::string{"lo"}));
  ASSERT_FALSE(android::base::EndsWith("goodbye", std::string{"lo"}));
}

TEST(strings, EndsWithIgnoreCase_std_string) {
  ASSERT_TRUE(android::base::EndsWithIgnoreCase("HeLlO", std::string{"lo"}));
  ASSERT_FALSE(android::base::EndsWithIgnoreCase("GoOdByE", std::string{"lo"}));
}

TEST(strings, EndsWith_char) {
  ASSERT_FALSE(android::base::EndsWith("", 'o'));
  ASSERT_TRUE(android::base::EndsWith("foo", 'o'));
  ASSERT_FALSE(android::base::EndsWith("foo", "f"));
}

TEST(strings, EqualsIgnoreCase) {
  ASSERT_TRUE(android::base::EqualsIgnoreCase("foo", "FOO"));
  ASSERT_TRUE(android::base::EqualsIgnoreCase("FOO", "foo"));
  ASSERT_FALSE(android::base::EqualsIgnoreCase("foo", "bar"));
  ASSERT_FALSE(android::base::EqualsIgnoreCase("foo", "fool"));
}

TEST(strings, ubsan_28729303) {
  android::base::Split("/dev/null", ":");
}

TEST(strings, ConsumePrefix) {
  std::string_view s{"foo.bar"};
  ASSERT_FALSE(android::base::ConsumePrefix(&s, "bar."));
  ASSERT_EQ("foo.bar", s);
  ASSERT_TRUE(android::base::ConsumePrefix(&s, "foo."));
  ASSERT_EQ("bar", s);
}

TEST(strings, ConsumeSuffix) {
  std::string_view s{"foo.bar"};
  ASSERT_FALSE(android::base::ConsumeSuffix(&s, ".foo"));
  ASSERT_EQ("foo.bar", s);
  ASSERT_TRUE(android::base::ConsumeSuffix(&s, ".bar"));
  ASSERT_EQ("foo", s);
}

TEST(strings, StringReplace_false) {
  // No change.
  ASSERT_EQ("abcabc", android::base::StringReplace("abcabc", "z", "Z", false));
  ASSERT_EQ("", android::base::StringReplace("", "z", "Z", false));
  ASSERT_EQ("abcabc", android::base::StringReplace("abcabc", "", "Z", false));

  // Equal lengths.
  ASSERT_EQ("Abcabc", android::base::StringReplace("abcabc", "a", "A", false));
  ASSERT_EQ("aBcabc", android::base::StringReplace("abcabc", "b", "B", false));
  ASSERT_EQ("abCabc", android::base::StringReplace("abcabc", "c", "C", false));

  // Longer replacement.
  ASSERT_EQ("foobcabc", android::base::StringReplace("abcabc", "a", "foo", false));
  ASSERT_EQ("afoocabc", android::base::StringReplace("abcabc", "b", "foo", false));
  ASSERT_EQ("abfooabc", android::base::StringReplace("abcabc", "c", "foo", false));

  // Shorter replacement.
  ASSERT_EQ("xxyz", android::base::StringReplace("abcxyz", "abc", "x", false));
  ASSERT_EQ("axyz", android::base::StringReplace("abcxyz", "bcx", "x", false));
  ASSERT_EQ("abcx", android::base::StringReplace("abcxyz", "xyz", "x", false));
}

TEST(strings, StringReplace_true) {
  // No change.
  ASSERT_EQ("abcabc", android::base::StringReplace("abcabc", "z", "Z", true));
  ASSERT_EQ("", android::base::StringReplace("", "z", "Z", true));
  ASSERT_EQ("abcabc", android::base::StringReplace("abcabc", "", "Z", true));

  // Equal lengths.
  ASSERT_EQ("AbcAbc", android::base::StringReplace("abcabc", "a", "A", true));
  ASSERT_EQ("aBcaBc", android::base::StringReplace("abcabc", "b", "B", true));
  ASSERT_EQ("abCabC", android::base::StringReplace("abcabc", "c", "C", true));

  // Longer replacement.
  ASSERT_EQ("foobcfoobc", android::base::StringReplace("abcabc", "a", "foo", true));
  ASSERT_EQ("afoocafooc", android::base::StringReplace("abcabc", "b", "foo", true));
  ASSERT_EQ("abfooabfoo", android::base::StringReplace("abcabc", "c", "foo", true));

  // Shorter replacement.
  ASSERT_EQ("xxyzx", android::base::StringReplace("abcxyzabc", "abc", "x", true));
  ASSERT_EQ("<xx>", android::base::StringReplace("<abcabc>", "abc", "x", true));
}
