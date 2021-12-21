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

#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

namespace android {
namespace base {

#define CHECK_NE(a, b) \
  if ((a) == (b)) abort();

std::vector<std::string> Split(const std::string& s,
                               const std::string& delimiters) {
  CHECK_NE(delimiters.size(), 0U);

  std::vector<std::string> result;

  size_t base = 0;
  size_t found;
  while (true) {
    found = s.find_first_of(delimiters, base);
    result.push_back(s.substr(base, found - base));
    if (found == s.npos) break;
    base = found + 1;
  }

  return result;
}

std::string Trim(const std::string& s) {
  std::string result;

  if (s.size() == 0) {
    return result;
  }

  size_t start_index = 0;
  size_t end_index = s.size() - 1;

  // Skip initial whitespace.
  while (start_index < s.size()) {
    if (!isspace(s[start_index])) {
      break;
    }
    start_index++;
  }

  // Skip terminating whitespace.
  while (end_index >= start_index) {
    if (!isspace(s[end_index])) {
      break;
    }
    end_index--;
  }

  // All spaces, no beef.
  if (end_index < start_index) {
    return "";
  }
  // Start_index is the first non-space, end_index is the last one.
  return s.substr(start_index, end_index - start_index + 1);
}

// These cases are probably the norm, so we mark them extern in the header to
// aid compile time and binary size.
template std::string Join(const std::vector<std::string>&, char);
template std::string Join(const std::vector<const char*>&, char);
template std::string Join(const std::vector<std::string>&, const std::string&);
template std::string Join(const std::vector<const char*>&, const std::string&);

bool StartsWith(std::string_view s, std::string_view prefix) {
  return s.substr(0, prefix.size()) == prefix;
}

bool StartsWith(std::string_view s, char prefix) {
  return !s.empty() && s.front() == prefix;
}

bool StartsWithIgnoreCase(std::string_view s, std::string_view prefix) {
  return s.size() >= prefix.size() && strncasecmp(s.data(), prefix.data(), prefix.size()) == 0;
}

bool EndsWith(std::string_view s, std::string_view suffix) {
  return s.size() >= suffix.size() && s.substr(s.size() - suffix.size(), suffix.size()) == suffix;
}

bool EndsWith(std::string_view s, char suffix) {
  return !s.empty() && s.back() == suffix;
}

bool EndsWithIgnoreCase(std::string_view s, std::string_view suffix) {
  return s.size() >= suffix.size() &&
         strncasecmp(s.data() + (s.size() - suffix.size()), suffix.data(), suffix.size()) == 0;
}

bool EqualsIgnoreCase(std::string_view lhs, std::string_view rhs) {
  return lhs.size() == rhs.size() && strncasecmp(lhs.data(), rhs.data(), lhs.size()) == 0;
}

std::string StringReplace(std::string_view s, std::string_view from, std::string_view to,
                          bool all) {
  if (from.empty()) return std::string(s);

  std::string result;
  std::string_view::size_type start_pos = 0;
  do {
    std::string_view::size_type pos = s.find(from, start_pos);
    if (pos == std::string_view::npos) break;

    result.append(s.data() + start_pos, pos - start_pos);
    result.append(to.data(), to.size());

    start_pos = pos + from.size();
  } while (all);
  result.append(s.data() + start_pos, s.size() - start_pos);
  return result;
}

}  // namespace base
}  // namespace android
