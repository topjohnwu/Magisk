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

#ifndef _BACKTRACE_BACKTRACE_H
#define _BACKTRACE_BACKTRACE_H

#include <inttypes.h>
#include <stdint.h>

#include <string>
#include <vector>

#include <backtrace/backtrace_constants.h>
#include <backtrace/BacktraceMap.h>

#if defined(__LP64__)
#define PRIPTR "016" PRIx64
typedef uint64_t word_t;
#else
#define PRIPTR "08" PRIx64
typedef uint32_t word_t;
#endif

enum BacktraceUnwindErrorCode : uint32_t {
  BACKTRACE_UNWIND_NO_ERROR,
  // Something failed while trying to perform the setup to begin the unwind.
  BACKTRACE_UNWIND_ERROR_SETUP_FAILED,
  // There is no map information to use with the unwind.
  BACKTRACE_UNWIND_ERROR_MAP_MISSING,
  // An error occurred that indicates a programming error.
  BACKTRACE_UNWIND_ERROR_INTERNAL,
  // The thread to unwind has disappeared before the unwind can begin.
  BACKTRACE_UNWIND_ERROR_THREAD_DOESNT_EXIST,
  // The thread to unwind has not responded to a signal in a timely manner.
  BACKTRACE_UNWIND_ERROR_THREAD_TIMEOUT,
  // Attempt to do an unsupported operation.
  BACKTRACE_UNWIND_ERROR_UNSUPPORTED_OPERATION,
  // Attempt to do an offline unwind without a context.
  BACKTRACE_UNWIND_ERROR_NO_CONTEXT,
  // The count of frames exceed MAX_BACKTRACE_FRAMES.
  BACKTRACE_UNWIND_ERROR_EXCEED_MAX_FRAMES_LIMIT,
  // Failed to read memory.
  BACKTRACE_UNWIND_ERROR_ACCESS_MEM_FAILED,
  // Failed to read registers.
  BACKTRACE_UNWIND_ERROR_ACCESS_REG_FAILED,
  // Failed to find a function in debug sections.
  BACKTRACE_UNWIND_ERROR_FIND_PROC_INFO_FAILED,
  // Failed to execute dwarf instructions in debug sections.
  BACKTRACE_UNWIND_ERROR_EXECUTE_DWARF_INSTRUCTION_FAILED,
  // Unwind information is incorrect.
  BACKTRACE_UNWIND_ERROR_UNWIND_INFO,
  // Unwind information stopped due to sp/pc repeating.
  BACKTRACE_UNWIND_ERROR_REPEATED_FRAME,
  // Unwind information stopped due to invalid elf.
  BACKTRACE_UNWIND_ERROR_INVALID_ELF,
};

struct BacktraceUnwindError {
  enum BacktraceUnwindErrorCode error_code;

  union {
    // for BACKTRACE_UNWIND_ERROR_ACCESS_MEM_FAILED
    uint64_t addr;
    // for BACKTRACE_UNWIND_ERROR_ACCESS_REG_FAILED
    uint64_t regno;
  } error_info;

  BacktraceUnwindError() : error_code(BACKTRACE_UNWIND_NO_ERROR) {}
};

struct backtrace_frame_data_t {
  size_t num;             // The current fame number.
  uint64_t pc;            // The absolute pc.
  uint64_t rel_pc;        // The relative pc.
  uint64_t sp;            // The top of the stack.
  size_t stack_size;      // The size of the stack, zero indicate an unknown stack size.
  backtrace_map_t map;    // The map associated with the given pc.
  std::string func_name;  // The function name associated with this pc, NULL if not found.
  uint64_t func_offset;  // pc relative to the start of the function, only valid if func_name is not
                         // NULL.
};

struct backtrace_stackinfo_t {
  uint64_t start;
  uint64_t end;
  const uint8_t* data;
};

namespace unwindstack {
class Regs;
}

class Backtrace {
 public:
  enum ArchEnum : uint8_t {
    ARCH_ARM,
    ARCH_ARM64,
    ARCH_X86,
    ARCH_X86_64,
  };

  static void SetGlobalElfCache(bool enable);

  // Create the correct Backtrace object based on what is to be unwound.
  // If pid < 0 or equals the current pid, then the Backtrace object
  // corresponds to the current process.
  // If pid < 0 or equals the current pid and tid >= 0, then the Backtrace
  // object corresponds to a thread in the current process.
  // If pid >= 0 and tid < 0, then the Backtrace object corresponds to a
  // different process.
  // Tracing a thread in a different process is not supported.
  // If map is NULL, then create the map and manage it internally.
  // If map is not NULL, the map is still owned by the caller.
  static Backtrace* Create(pid_t pid, pid_t tid, BacktraceMap* map = nullptr);

  virtual ~Backtrace();

  // Get the current stack trace and store in the backtrace_ structure.
  virtual bool Unwind(size_t num_ignore_frames, void* context = nullptr) = 0;

  static bool Unwind(unwindstack::Regs* regs, BacktraceMap* back_map,
                     std::vector<backtrace_frame_data_t>* frames, size_t num_ignore_frames,
                     std::vector<std::string>* skip_names, BacktraceUnwindError* error = nullptr);

  // Get the function name and offset into the function given the pc.
  // If the string is empty, then no valid function name was found,
  // or the pc is not in any valid map.
  virtual std::string GetFunctionName(uint64_t pc, uint64_t* offset,
                                      const backtrace_map_t* map = nullptr);

  // Fill in the map data associated with the given pc.
  virtual void FillInMap(uint64_t pc, backtrace_map_t* map);

  // Read the data at a specific address.
  virtual bool ReadWord(uint64_t ptr, word_t* out_value) = 0;

  // Read arbitrary data from a specific address. If a read request would
  // span from one map to another, this call only reads up until the end
  // of the current map.
  // Returns the total number of bytes actually read.
  virtual size_t Read(uint64_t addr, uint8_t* buffer, size_t bytes) = 0;

  // Create a string representing the formatted line of backtrace information
  // for a single frame.
  virtual std::string FormatFrameData(size_t frame_num);
  static std::string FormatFrameData(const backtrace_frame_data_t* frame);

  pid_t Pid() const { return pid_; }
  pid_t Tid() const { return tid_; }
  size_t NumFrames() const { return frames_.size(); }

  const backtrace_frame_data_t* GetFrame(size_t frame_num) {
    if (frame_num >= frames_.size()) {
      return nullptr;
    }
    return &frames_[frame_num];
  }

  typedef std::vector<backtrace_frame_data_t>::iterator iterator;
  iterator begin() { return frames_.begin(); }
  iterator end() { return frames_.end(); }

  typedef std::vector<backtrace_frame_data_t>::const_iterator const_iterator;
  const_iterator begin() const { return frames_.begin(); }
  const_iterator end() const { return frames_.end(); }

  BacktraceMap* GetMap() { return map_; }

  BacktraceUnwindError GetError() { return error_; }

  std::string GetErrorString(BacktraceUnwindError error);

  // Set whether to skip frames in libbacktrace/libunwindstack when doing a local unwind.
  void SetSkipFrames(bool skip_frames) { skip_frames_ = skip_frames; }

 protected:
  Backtrace(pid_t pid, pid_t tid, BacktraceMap* map);

  // The name returned is not demangled, GetFunctionName() takes care of
  // demangling the name.
  virtual std::string GetFunctionNameRaw(uint64_t pc, uint64_t* offset) = 0;

  virtual bool VerifyReadWordArgs(uint64_t ptr, word_t* out_value);

  bool BuildMap();

  pid_t pid_;
  pid_t tid_;

  BacktraceMap* map_;
  bool map_shared_;

  std::vector<backtrace_frame_data_t> frames_;

  // Skip frames in libbacktrace/libunwindstack when doing a local unwind.
  bool skip_frames_ = true;

  BacktraceUnwindError error_;
};

#endif // _BACKTRACE_BACKTRACE_H
