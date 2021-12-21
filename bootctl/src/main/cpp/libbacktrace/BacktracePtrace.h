/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef _LIBBACKTRACE_BACKTRACE_PTRACE_H
#define _LIBBACKTRACE_BACKTRACE_PTRACE_H

#include <stdint.h>
#include <sys/types.h>

#include <backtrace/Backtrace.h>

class BacktraceMap;

class BacktracePtrace : public Backtrace {
 public:
  BacktracePtrace(pid_t pid, pid_t tid, BacktraceMap* map) : Backtrace(pid, tid, map) {}
  virtual ~BacktracePtrace() {}

  size_t Read(uint64_t addr, uint8_t* buffer, size_t bytes) override;

  bool ReadWord(uint64_t ptr, word_t* out_value) override;
};

#endif // _LIBBACKTRACE_BACKTRACE_PTRACE_H
