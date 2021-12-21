/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef _BACKTRACE_BACKTRACE_MAP_H
#define _BACKTRACE_BACKTRACE_MAP_H

#include <stdint.h>
#include <sys/types.h>
#ifdef _WIN32
// MINGW does not define these constants.
#define PROT_NONE 0
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#else
#include <sys/mman.h>
#endif

#include <deque>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

// Forward declaration.
struct backtrace_stackinfo_t;

// Special flag to indicate a map is in /dev/. However, a map in
// /dev/ashmem/... does not set this flag.
static constexpr int PROT_DEVICE_MAP = 0x8000;
// Special flag to indicate that this map represents an elf file
// created by ART for use with the gdb jit debug interface.
// This should only ever appear in offline maps data.
static constexpr int PROT_JIT_SYMFILE_MAP = 0x4000;

struct backtrace_map_t {
  uint64_t start = 0;
  uint64_t end = 0;
  uint64_t offset = 0;
  uint64_t load_bias = 0;
  int flags = 0;
  std::string name;

  // Returns `name` if non-empty, or `<anonymous:0x...>` otherwise.
  std::string Name() const;
};

namespace unwindstack {
class Memory;
}

class BacktraceMap {
public:
  // If uncached is true, then parse the current process map as of the call.
  // Passing a map created with uncached set to true to Backtrace::Create()
  // is unsupported.
  static BacktraceMap* Create(pid_t pid, bool uncached = false);

  virtual ~BacktraceMap();

  class iterator : public std::iterator<std::bidirectional_iterator_tag, backtrace_map_t*> {
   public:
    iterator(BacktraceMap* map, size_t index) : map_(map), index_(index) {}

    iterator& operator++() {
      index_++;
      return *this;
    }
    const iterator operator++(int increment) {
      index_ += increment;
      return *this;
    }
    iterator& operator--() {
      index_--;
      return *this;
    }
    const iterator operator--(int decrement) {
      index_ -= decrement;
      return *this;
    }

    bool operator==(const iterator& rhs) { return this->index_ == rhs.index_; }
    bool operator!=(const iterator& rhs) { return this->index_ != rhs.index_; }

    const backtrace_map_t* operator*() {
      if (index_ >= map_->size()) {
        return nullptr;
      }
      backtrace_map_t* map = &map_->maps_[index_];
      if (map->load_bias == static_cast<uint64_t>(-1)) {
        map->load_bias = map_->GetLoadBias(index_);
      }
      return map;
    }

   private:
    BacktraceMap* map_ = nullptr;
    size_t index_ = 0;
  };

  iterator begin() { return iterator(this, 0); }
  iterator end() { return iterator(this, maps_.size()); }

  // Fill in the map data structure for the given address.
  virtual void FillIn(uint64_t addr, backtrace_map_t* map);

  // Only supported with the new unwinder.
  virtual std::string GetFunctionName(uint64_t /*pc*/, uint64_t* /*offset*/) { return ""; }
  virtual std::shared_ptr<unwindstack::Memory> GetProcessMemory() { return nullptr; }

  // The flags returned are the same flags as used by the mmap call.
  // The values are PROT_*.
  int GetFlags(uint64_t pc) {
    backtrace_map_t map;
    FillIn(pc, &map);
    if (IsValid(map)) {
      return map.flags;
    }
    return PROT_NONE;
  }

  bool IsReadable(uint64_t pc) { return GetFlags(pc) & PROT_READ; }
  bool IsWritable(uint64_t pc) { return GetFlags(pc) & PROT_WRITE; }
  bool IsExecutable(uint64_t pc) { return GetFlags(pc) & PROT_EXEC; }

  // In order to use the iterators on this object, a caller must
  // call the LockIterator and UnlockIterator function to guarantee
  // that the data does not change while it's being used.
  virtual void LockIterator() {}
  virtual void UnlockIterator() {}

  size_t size() const { return maps_.size(); }

  virtual bool Build();

  static inline bool IsValid(const backtrace_map_t& map) {
    return map.end > 0;
  }

  void SetSuffixesToIgnore(std::vector<std::string> suffixes) {
    suffixes_to_ignore_.insert(suffixes_to_ignore_.end(), suffixes.begin(), suffixes.end());
  }

  const std::vector<std::string>& GetSuffixesToIgnore() { return suffixes_to_ignore_; }

  // Disabling the resolving of names results in the function name being
  // set to an empty string and the function offset being set to zero
  // in the frame data when unwinding.
  void SetResolveNames(bool resolve) { resolve_names_ = resolve; }

  bool ResolveNames() { return resolve_names_; }

 protected:
  BacktraceMap(pid_t pid);

  virtual uint64_t GetLoadBias(size_t /* index */) { return 0; }

  pid_t pid_;
  std::deque<backtrace_map_t> maps_;
  std::vector<std::string> suffixes_to_ignore_;
  bool resolve_names_ = true;
};

class ScopedBacktraceMapIteratorLock {
public:
  explicit ScopedBacktraceMapIteratorLock(BacktraceMap* map) : map_(map) {
    map->LockIterator();
  }

  ~ScopedBacktraceMapIteratorLock() {
    map_->UnlockIterator();
  }

private:
  BacktraceMap* map_;
};

#endif // _BACKTRACE_BACKTRACE_MAP_H
