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

#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace android {
namespace base {

// Splits a string into a vector of strings.
//
// The string is split at each occurrence of a character in delimiters.
//
// The empty string is not a valid delimiter list.
std::vector<std::string> Split(const std::string& s,
                               const std::string& delimiters);

// Trims whitespace off both ends of the given string.
std::string Trim(const std::string& s);

// Joins a container of things into a single string, using the given separator.
template <typename ContainerT, typename SeparatorT>
std::string Join(const ContainerT& things, SeparatorT separator) {
  if (things.empty()) {
    return "";
  }

  std::ostringstream result;
  result << *things.begin();
  for (auto it = std::next(things.begin()); it != things.end(); ++it) {
    result << separator << *it;
  }
  return result.str();
}

// We instantiate the common cases in strings.cpp.
extern template std::string Join(const std::vector<std::string>&, char);
extern template std::string Join(const std::vector<const char*>&, char);
extern template std::string Join(const std::vector<std::string>&, const std::string&);
extern template std::string Join(const std::vector<const char*>&, const std::string&);

// Tests whether 's' starts with 'prefix'.
bool StartsWith(std::string_view s, std::string_view prefix);
bool StartsWith(std::string_view s, char prefix);
bool StartsWithIgnoreCase(std::string_view s, std::string_view prefix);

// Tests whether 's' ends with 'suffix'.
bool EndsWith(std::string_view s, std::string_view suffix);
bool EndsWith(std::string_view s, char suffix);
bool EndsWithIgnoreCase(std::string_view s, std::string_view suffix);

// Tests whether 'lhs' equals 'rhs', ignoring case.
bool EqualsIgnoreCase(std::string_view lhs, std::string_view rhs);

// Removes `prefix` from the start of the given string and returns true (if
// it was present), false otherwise.
inline bool ConsumePrefix(std::string_view* s, std::string_view prefix) {
  if (!StartsWith(*s, prefix)) return false;
  s->remove_prefix(prefix.size());
  return true;
}

// Removes `suffix` from the end of the given string and returns true (if
// it was present), false otherwise.
inline bool ConsumeSuffix(std::string_view* s, std::string_view suffix) {
  if (!EndsWith(*s, suffix)) return false;
  s->remove_suffix(suffix.size());
  return true;
}

// Replaces `from` with `to` in `s`, once if `all == false`, or as many times as
// there are matches if `all == true`.
[[nodiscard]] std::string StringReplace(std::string_view s, std::string_view from,
                                        std::string_view to, bool all);

}  // namespace base
}  // namespace android
