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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <limits>
#include <string>
#include <type_traits>

namespace android {
namespace base {

// Parses the unsigned decimal or hexadecimal integer in the string 's' and sets
// 'out' to that value if it is specified. Optionally allows the caller to define
// a 'max' beyond which otherwise valid values will be rejected. Returns boolean
// success; 'out' is untouched if parsing fails.
template <typename T>
bool ParseUint(const char* s, T* out, T max = std::numeric_limits<T>::max(),
               bool allow_suffixes = false) {
  static_assert(std::is_unsigned<T>::value, "ParseUint can only be used with unsigned types");
  while (isspace(*s)) {
    s++;
  }

  if (s[0] == '-') {
    errno = EINVAL;
    return false;
  }

  int base = (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) ? 16 : 10;
  errno = 0;
  char* end;
  unsigned long long int result = strtoull(s, &end, base);
  if (errno != 0) return false;
  if (end == s) {
    errno = EINVAL;
    return false;
  }
  if (*end != '\0') {
    const char* suffixes = "bkmgtpe";
    const char* suffix;
    if ((!allow_suffixes || (suffix = strchr(suffixes, tolower(*end))) == nullptr) ||
        __builtin_mul_overflow(result, 1ULL << (10 * (suffix - suffixes)), &result)) {
      errno = EINVAL;
      return false;
    }
  }
  if (max < result) {
    errno = ERANGE;
    return false;
  }
  if (out != nullptr) {
    *out = static_cast<T>(result);
  }
  return true;
}

// TODO: string_view
template <typename T>
bool ParseUint(const std::string& s, T* out, T max = std::numeric_limits<T>::max(),
               bool allow_suffixes = false) {
  return ParseUint(s.c_str(), out, max, allow_suffixes);
}

template <typename T>
bool ParseByteCount(const char* s, T* out, T max = std::numeric_limits<T>::max()) {
  return ParseUint(s, out, max, true);
}

// TODO: string_view
template <typename T>
bool ParseByteCount(const std::string& s, T* out, T max = std::numeric_limits<T>::max()) {
  return ParseByteCount(s.c_str(), out, max);
}

// Parses the signed decimal or hexadecimal integer in the string 's' and sets
// 'out' to that value if it is specified. Optionally allows the caller to define
// a 'min' and 'max' beyond which otherwise valid values will be rejected. Returns
// boolean success; 'out' is untouched if parsing fails.
template <typename T>
bool ParseInt(const char* s, T* out,
              T min = std::numeric_limits<T>::min(),
              T max = std::numeric_limits<T>::max()) {
  static_assert(std::is_signed<T>::value, "ParseInt can only be used with signed types");
  while (isspace(*s)) {
    s++;
  }

  int base = (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) ? 16 : 10;
  errno = 0;
  char* end;
  long long int result = strtoll(s, &end, base);
  if (errno != 0) {
    return false;
  }
  if (s == end || *end != '\0') {
    errno = EINVAL;
    return false;
  }
  if (result < min || max < result) {
    errno = ERANGE;
    return false;
  }
  if (out != nullptr) {
    *out = static_cast<T>(result);
  }
  return true;
}

// TODO: string_view
template <typename T>
bool ParseInt(const std::string& s, T* out,
              T min = std::numeric_limits<T>::min(),
              T max = std::numeric_limits<T>::max()) {
  return ParseInt(s.c_str(), out, min, max);
}

}  // namespace base
}  // namespace android
