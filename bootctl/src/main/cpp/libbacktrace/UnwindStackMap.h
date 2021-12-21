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

#ifndef _LIBBACKTRACE_UNWINDSTACK_MAP_H
#define _LIBBACKTRACE_UNWINDSTACK_MAP_H

#include <stdint.h>
#include <sys/types.h>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <backtrace/Backtrace.h>
#include <backtrace/BacktraceMap.h>
#if !defined(NO_LIBDEXFILE_SUPPORT)
#include <unwindstack/DexFiles.h>
#endif
#include <unwindstack/Elf.h>
#include <unwindstack/JitDebug.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>

// Forward declarations.
class UnwindDexFile;

class UnwindStackMap : public BacktraceMap {
 public:
  explicit UnwindStackMap(pid_t pid);
  ~UnwindStackMap() = default;

  bool Build() override;

  void FillIn(uint64_t addr, backtrace_map_t* map) override;

  virtual std::string GetFunctionName(uint64_t pc, uint64_t* offset) override;
  virtual std::shared_ptr<unwindstack::Memory> GetProcessMemory() override final;

  unwindstack::Maps* stack_maps() { return stack_maps_.get(); }

  const std::shared_ptr<unwindstack::Memory>& process_memory() { return process_memory_; }

  unwindstack::JitDebug* GetJitDebug() { return jit_debug_.get(); }

#if !defined(NO_LIBDEXFILE_SUPPORT)
  unwindstack::DexFiles* GetDexFiles() { return dex_files_.get(); }
#endif

  void SetArch(unwindstack::ArchEnum arch) { arch_ = arch; }

 protected:
  uint64_t GetLoadBias(size_t index) override;

  std::unique_ptr<unwindstack::Maps> stack_maps_;
  std::shared_ptr<unwindstack::Memory> process_memory_;
  std::unique_ptr<unwindstack::JitDebug> jit_debug_;
#if !defined(NO_LIBDEXFILE_SUPPORT)
  std::unique_ptr<unwindstack::DexFiles> dex_files_;
#endif

  unwindstack::ArchEnum arch_ = unwindstack::ARCH_UNKNOWN;
};

#endif  // _LIBBACKTRACE_UNWINDSTACK_MAP_H
