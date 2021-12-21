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

#pragma once

#include <dirent.h>
#include <sys/types.h>

#include <iterator>
#include <memory>
#include <vector>

namespace android {
namespace base {

class AllPids {
  class PidIterator {
   public:
    PidIterator(DIR* dir) : dir_(dir, closedir) { Increment(); }
    PidIterator& operator++() {
      Increment();
      return *this;
    }
    bool operator==(const PidIterator& other) const { return pid_ == other.pid_; }
    bool operator!=(const PidIterator& other) const { return !(*this == other); }
    long operator*() const { return pid_; }
    // iterator traits
    using difference_type = pid_t;
    using value_type = pid_t;
    using pointer = const pid_t*;
    using reference = const pid_t&;
    using iterator_category = std::input_iterator_tag;

   private:
    void Increment();

    pid_t pid_ = -1;
    std::unique_ptr<DIR, decltype(&closedir)> dir_;
  };

 public:
  PidIterator begin() { return opendir("/proc"); }
  PidIterator end() { return nullptr; }
};

}  // namespace base
}  // namespace android
