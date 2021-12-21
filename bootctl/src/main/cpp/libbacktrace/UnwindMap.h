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

#ifndef _LIBBACKTRACE_UNWIND_MAP_H
#define _LIBBACKTRACE_UNWIND_MAP_H

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>

#include <backtrace/BacktraceMap.h>

// The unw_map_cursor_t structure is different depending on whether it is
// the local or remote version. In order to get the correct version, include
// libunwind.h first then this header.

class UnwindMap : public BacktraceMap {
 public:
  explicit UnwindMap(pid_t pid);

  unw_map_cursor_t* GetMapCursor() { return &map_cursor_; }

 protected:
  unw_map_cursor_t map_cursor_;
};

class UnwindMapRemote : public UnwindMap {
 public:
  explicit UnwindMapRemote(pid_t pid);
  virtual ~UnwindMapRemote();

  bool Build() override;

 private:
  bool GenerateMap();
};

class UnwindMapLocal : public UnwindMap {
 public:
  UnwindMapLocal();
  virtual ~UnwindMapLocal();

  bool Build() override;

  void FillIn(uint64_t addr, backtrace_map_t* map) override;

  void LockIterator() override { pthread_rwlock_rdlock(&map_lock_); }
  void UnlockIterator() override { pthread_rwlock_unlock(&map_lock_); }

 private:
  bool GenerateMap();

  bool map_created_;

  pthread_rwlock_t map_lock_;
};

#endif // _LIBBACKTRACE_UNWIND_MAP_H
