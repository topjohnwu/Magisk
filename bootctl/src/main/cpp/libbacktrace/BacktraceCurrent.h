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

#ifndef _LIBBACKTRACE_BACKTRACE_CURRENT_H
#define _LIBBACKTRACE_BACKTRACE_CURRENT_H

#include <stdint.h>
#include <sys/types.h>

#include <backtrace/Backtrace.h>

// The signal used to cause a thread to dump the stack.
#if defined(__GLIBC__)
// In order to run the backtrace_tests on the host, we can't use
// the internal real time signals used by GLIBC. To avoid this,
// use SIGRTMIN for the signal to dump the stack.
#define THREAD_SIGNAL SIGRTMIN
#else
#define THREAD_SIGNAL (__SIGRTMIN+1)
#endif

class BacktraceMap;

class BacktraceCurrent : public Backtrace {
 public:
  BacktraceCurrent(pid_t pid, pid_t tid, BacktraceMap* map) : Backtrace(pid, tid, map) {}
  virtual ~BacktraceCurrent() {}

  size_t Read(uint64_t addr, uint8_t* buffer, size_t bytes) override;

  bool ReadWord(uint64_t ptr, word_t* out_value) override;

  bool Unwind(size_t num_ignore_frames, void* ucontext) override;

 protected:
  bool DiscardFrame(const backtrace_frame_data_t& frame);

 private:
  bool UnwindThread(size_t num_ignore_frames);

  virtual bool UnwindFromContext(size_t num_ignore_frames, void* ucontext) = 0;
};

#endif // _LIBBACKTRACE_BACKTRACE_CURRENT_H
