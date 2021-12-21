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

#pragma once

#include <errno.h>
#include <stdlib.h>

#include <limits>
#include <string>

namespace android {
namespace base {

// Parse floating value in the string 's' and sets 'out' to that value if it exists.
// Optionally allows the caller to define a 'min' and 'max' beyond which
// otherwise valid values will be rejected. Returns boolean success.
template <typename T, T (*strtox)(const char* str, char** endptr)>
static inline bool ParseFloatingPoint(const char* s, T* out, T min, T max) {
  errno = 0;
  char* end;
  T result = strtox(s, &end);
  if (errno != 0 || s == end || *end != '\0') {
    return false;
  }
  if (result < min || max < result) {
    return false;
  }
  if (out != nullptr) {
    *out = result;
  }
  return true;
}

// Parse double value in the string 's' and sets 'out' to that value if it exists.
// Optionally allows the caller to define a 'min' and 'max' beyond which
// otherwise valid values will be rejected. Returns boolean success.
static inline bool ParseDouble(const char* s, double* out,
                               double min = std::numeric_limits<double>::lowest(),
                               double max = std::numeric_limits<double>::max()) {
  return ParseFloatingPoint<double, strtod>(s, out, min, max);
}
static inline bool ParseDouble(const std::string& s, double* out,
                               double min = std::numeric_limits<double>::lowest(),
                               double max = std::numeric_limits<double>::max()) {
  return ParseFloatingPoint<double, strtod>(s.c_str(), out, min, max);
}

// Parse float value in the string 's' and sets 'out' to that value if it exists.
// Optionally allows the caller to define a 'min' and 'max' beyond which
// otherwise valid values will be rejected. Returns boolean success.
static inline bool ParseFloat(const char* s, float* out,
                              float min = std::numeric_limits<float>::lowest(),
                              float max = std::numeric_limits<float>::max()) {
  return ParseFloatingPoint<float, strtof>(s, out, min, max);
}
static inline bool ParseFloat(const std::string& s, float* out,
                              float min = std::numeric_limits<float>::lowest(),
                              float max = std::numeric_limits<float>::max()) {
  return ParseFloatingPoint<float, strtof>(s.c_str(), out, min, max);
}

}  // namespace base
}  // namespace android
