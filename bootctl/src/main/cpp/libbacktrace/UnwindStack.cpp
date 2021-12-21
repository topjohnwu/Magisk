/*
 * Copyright (C) 2017 The Android Open Source Project
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

#define _GNU_SOURCE 1
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <set>
#include <string>

#include <backtrace/Backtrace.h>
#include <unwindstack/Elf.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsGetLocal.h>

#if !defined(NO_LIBDEXFILE_SUPPORT)
#include <unwindstack/DexFiles.h>
#endif
#include <unwindstack/Unwinder.h>

#include "BacktraceLog.h"
#include "UnwindStack.h"
#include "UnwindStackMap.h"

extern "C" char* __cxa_demangle(const char*, char*, size_t*, int*);

bool Backtrace::Unwind(unwindstack::Regs* regs, BacktraceMap* back_map,
                       std::vector<backtrace_frame_data_t>* frames, size_t num_ignore_frames,
                       std::vector<std::string>* skip_names, BacktraceUnwindError* error) {
  UnwindStackMap* stack_map = reinterpret_cast<UnwindStackMap*>(back_map);
  auto process_memory = stack_map->process_memory();
  unwindstack::Unwinder unwinder(MAX_BACKTRACE_FRAMES + num_ignore_frames, stack_map->stack_maps(),
                                 regs, stack_map->process_memory());
  unwinder.SetResolveNames(stack_map->ResolveNames());
  stack_map->SetArch(regs->Arch());
  if (stack_map->GetJitDebug() != nullptr) {
    unwinder.SetJitDebug(stack_map->GetJitDebug());
  }
#if !defined(NO_LIBDEXFILE_SUPPORT)
  if (stack_map->GetDexFiles() != nullptr) {
    unwinder.SetDexFiles(stack_map->GetDexFiles());
  }
#endif
  unwinder.Unwind(skip_names, &stack_map->GetSuffixesToIgnore());
  if (error != nullptr) {
    switch (unwinder.LastErrorCode()) {
      case unwindstack::ERROR_NONE:
        error->error_code = BACKTRACE_UNWIND_NO_ERROR;
        break;

      case unwindstack::ERROR_MEMORY_INVALID:
        error->error_code = BACKTRACE_UNWIND_ERROR_ACCESS_MEM_FAILED;
        error->error_info.addr = unwinder.LastErrorAddress();
        break;

      case unwindstack::ERROR_UNWIND_INFO:
        error->error_code = BACKTRACE_UNWIND_ERROR_UNWIND_INFO;
        break;

      case unwindstack::ERROR_UNSUPPORTED:
        error->error_code = BACKTRACE_UNWIND_ERROR_UNSUPPORTED_OPERATION;
        break;

      case unwindstack::ERROR_INVALID_MAP:
        error->error_code = BACKTRACE_UNWIND_ERROR_MAP_MISSING;
        break;

      case unwindstack::ERROR_MAX_FRAMES_EXCEEDED:
        error->error_code = BACKTRACE_UNWIND_ERROR_EXCEED_MAX_FRAMES_LIMIT;
        break;

      case unwindstack::ERROR_REPEATED_FRAME:
        error->error_code = BACKTRACE_UNWIND_ERROR_REPEATED_FRAME;
        break;

      case unwindstack::ERROR_INVALID_ELF:
        error->error_code = BACKTRACE_UNWIND_ERROR_INVALID_ELF;
        break;

      case unwindstack::ERROR_SYSTEM_CALL:
        error->error_code = BACKTRACE_UNWIND_ERROR_INTERNAL;
        break;

      case unwindstack::ERROR_THREAD_DOES_NOT_EXIST:
        error->error_code = BACKTRACE_UNWIND_ERROR_THREAD_DOESNT_EXIST;
        break;

      case unwindstack::ERROR_THREAD_TIMEOUT:
        error->error_code = BACKTRACE_UNWIND_ERROR_THREAD_TIMEOUT;
        break;
    }
  }

  if (num_ignore_frames >= unwinder.NumFrames()) {
    frames->resize(0);
    return true;
  }

  auto unwinder_frames = unwinder.frames();
  frames->resize(unwinder.NumFrames() - num_ignore_frames);
  size_t cur_frame = 0;
  for (size_t i = num_ignore_frames; i < unwinder.NumFrames(); i++) {
    auto frame = &unwinder_frames[i];

    backtrace_frame_data_t* back_frame = &frames->at(cur_frame);

    back_frame->num = cur_frame++;

    back_frame->rel_pc = frame->rel_pc;
    back_frame->pc = frame->pc;
    back_frame->sp = frame->sp;

    char* demangled_name = __cxa_demangle(frame->function_name.c_str(), nullptr, nullptr, nullptr);
    if (demangled_name != nullptr) {
      back_frame->func_name = demangled_name;
      free(demangled_name);
    } else {
      back_frame->func_name = frame->function_name;
    }
    back_frame->func_offset = frame->function_offset;

    back_frame->map.name = frame->map_name;
    back_frame->map.start = frame->map_start;
    back_frame->map.end = frame->map_end;
    back_frame->map.offset = frame->map_elf_start_offset;
    back_frame->map.load_bias = frame->map_load_bias;
    back_frame->map.flags = frame->map_flags;
  }

  return true;
}

UnwindStackCurrent::UnwindStackCurrent(pid_t pid, pid_t tid, BacktraceMap* map)
    : BacktraceCurrent(pid, tid, map) {}

std::string UnwindStackCurrent::GetFunctionNameRaw(uint64_t pc, uint64_t* offset) {
  return GetMap()->GetFunctionName(pc, offset);
}

bool UnwindStackCurrent::UnwindFromContext(size_t num_ignore_frames, void* ucontext) {
  std::unique_ptr<unwindstack::Regs> regs;
  if (ucontext == nullptr) {
    regs.reset(unwindstack::Regs::CreateFromLocal());
    // Fill in the registers from this function. Do it here to avoid
    // one extra function call appearing in the unwind.
    unwindstack::RegsGetLocal(regs.get());
  } else {
    regs.reset(unwindstack::Regs::CreateFromUcontext(unwindstack::Regs::CurrentArch(), ucontext));
  }

  std::vector<std::string> skip_names{"libunwindstack.so", "libbacktrace.so"};
  if (!skip_frames_) {
    skip_names.clear();
  }
  return Backtrace::Unwind(regs.get(), GetMap(), &frames_, num_ignore_frames, &skip_names, &error_);
}

UnwindStackPtrace::UnwindStackPtrace(pid_t pid, pid_t tid, BacktraceMap* map)
    : BacktracePtrace(pid, tid, map), memory_(unwindstack::Memory::CreateProcessMemory(pid)) {}

std::string UnwindStackPtrace::GetFunctionNameRaw(uint64_t pc, uint64_t* offset) {
  return GetMap()->GetFunctionName(pc, offset);
}

bool UnwindStackPtrace::Unwind(size_t num_ignore_frames, void* context) {
  std::unique_ptr<unwindstack::Regs> regs;
  if (context == nullptr) {
    regs.reset(unwindstack::Regs::RemoteGet(Tid()));
  } else {
    regs.reset(unwindstack::Regs::CreateFromUcontext(unwindstack::Regs::CurrentArch(), context));
  }

  return Backtrace::Unwind(regs.get(), GetMap(), &frames_, num_ignore_frames, nullptr, &error_);
}

size_t UnwindStackPtrace::Read(uint64_t addr, uint8_t* buffer, size_t bytes) {
#if defined(__aarch64__)
  // Tagged pointer after Android R would lead top byte to have random values
  // https://source.android.com/devices/tech/debug/tagged-pointers
  addr &= (1ULL << 56) - 1;
#endif
  return memory_->Read(addr, buffer, bytes);
}
