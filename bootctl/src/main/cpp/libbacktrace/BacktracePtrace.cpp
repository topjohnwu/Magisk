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

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>

#include <backtrace/Backtrace.h>
#include <backtrace/BacktraceMap.h>

#include "BacktraceLog.h"
#include "BacktracePtrace.h"

#if !defined(__APPLE__)
static bool PtraceRead(pid_t tid, uint64_t addr, word_t* out_value) {
  // ptrace() returns -1 and sets errno when the operation fails.
  // To disambiguate -1 from a valid result, we clear errno beforehand.
  errno = 0;
  *out_value = ptrace(PTRACE_PEEKTEXT, tid, reinterpret_cast<void*>(addr), nullptr);
  if (*out_value == static_cast<word_t>(-1) && errno) {
    return false;
  }
  return true;
}
#endif

bool BacktracePtrace::ReadWord(uint64_t ptr, word_t* out_value) {
#if defined(__APPLE__)
  BACK_LOGW("MacOS does not support reading from another pid.");
  return false;
#else
  if (!VerifyReadWordArgs(ptr, out_value)) {
    return false;
  }

  backtrace_map_t map;
  FillInMap(ptr, &map);
  if (!BacktraceMap::IsValid(map) || !(map.flags & PROT_READ)) {
    return false;
  }

  return PtraceRead(Tid(), ptr, out_value);
#endif
}

size_t BacktracePtrace::Read(uint64_t addr, uint8_t* buffer, size_t bytes) {
#if defined(__APPLE__)
  BACK_LOGW("MacOS does not support reading from another pid.");
  return 0;
#else
  backtrace_map_t map;
  FillInMap(addr, &map);
  if (!BacktraceMap::IsValid(map) || !(map.flags & PROT_READ)) {
    return 0;
  }

  bytes = MIN(map.end - addr, bytes);
  size_t bytes_read = 0;
  word_t data_word;
  size_t align_bytes = addr & (sizeof(word_t) - 1);
  if (align_bytes != 0) {
    if (!PtraceRead(Tid(), addr & ~(sizeof(word_t) - 1), &data_word)) {
      return 0;
    }
    size_t copy_bytes = MIN(sizeof(word_t) - align_bytes, bytes);
    memcpy(buffer, reinterpret_cast<uint8_t*>(&data_word) + align_bytes, copy_bytes);
    addr += copy_bytes;
    buffer += copy_bytes;
    bytes -= copy_bytes;
    bytes_read += copy_bytes;
  }

  size_t num_words = bytes / sizeof(word_t);
  for (size_t i = 0; i < num_words; i++) {
    if (!PtraceRead(Tid(), addr, &data_word)) {
      return bytes_read;
    }
    memcpy(buffer, &data_word, sizeof(word_t));
    buffer += sizeof(word_t);
    addr += sizeof(word_t);
    bytes_read += sizeof(word_t);
  }

  size_t left_over = bytes & (sizeof(word_t) - 1);
  if (left_over) {
    if (!PtraceRead(Tid(), addr, &data_word)) {
      return bytes_read;
    }
    memcpy(buffer, &data_word, left_over);
    bytes_read += left_over;
  }
  return bytes_read;
#endif
}
