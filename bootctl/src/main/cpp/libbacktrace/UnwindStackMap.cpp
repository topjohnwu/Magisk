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

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include <string>
#include <vector>

#include <backtrace/BacktraceMap.h>
#include <unwindstack/Elf.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Regs.h>

#include "UnwindStackMap.h"

//-------------------------------------------------------------------------
UnwindStackMap::UnwindStackMap(pid_t pid) : BacktraceMap(pid) {}

bool UnwindStackMap::Build() {
  if (pid_ == 0) {
    pid_ = getpid();
    stack_maps_.reset(new unwindstack::LocalMaps);
  } else {
    stack_maps_.reset(new unwindstack::RemoteMaps(pid_));
  }

  // Create the process memory object.
  process_memory_ = unwindstack::Memory::CreateProcessMemory(pid_);

  if (arch_ == unwindstack::ARCH_UNKNOWN) {
    if (pid_ == getpid()) {
      arch_ = unwindstack::Regs::CurrentArch();
    } else {
      // Create a remote regs, to figure out the architecture.
      std::unique_ptr<unwindstack::Regs> regs(unwindstack::Regs::RemoteGet(pid_));
      if (regs.get() != nullptr) {
        arch_ = regs->Arch();
      }
    }
  }

  // Create a JitDebug object for getting jit unwind information.
  if (arch_ != unwindstack::ARCH_UNKNOWN) {
    std::vector<std::string> search_libs_{"libart.so", "libartd.so"};
    jit_debug_ = CreateJitDebug(arch_, process_memory_, search_libs_);
#if !defined(NO_LIBDEXFILE_SUPPORT)
    dex_files_ = CreateDexFiles(arch_, process_memory_, search_libs_);
#endif
  }

  if (!stack_maps_->Parse()) {
    return false;
  }

  // Iterate through the maps and fill in the backtrace_map_t structure.
  for (const auto& map_info : *stack_maps_) {
    backtrace_map_t map;
    map.start = map_info->start();
    map.end = map_info->end();
    map.offset = map_info->offset();
    // Set to -1 so that it is demand loaded.
    map.load_bias = static_cast<uint64_t>(-1);
    map.flags = map_info->flags();
    map.name = map_info->name();

    maps_.push_back(map);
  }

  return true;
}

void UnwindStackMap::FillIn(uint64_t addr, backtrace_map_t* map) {
  BacktraceMap::FillIn(addr, map);
  if (map->load_bias != static_cast<uint64_t>(-1)) {
    return;
  }

  // Fill in the load_bias.
  unwindstack::MapInfo* map_info = stack_maps_->Find(addr);
  if (map_info == nullptr) {
    return;
  }
  map->load_bias = map_info->GetLoadBias(process_memory_);
}

uint64_t UnwindStackMap::GetLoadBias(size_t index) {
  if (index >= stack_maps_->Total()) {
    return 0;
  }

  unwindstack::MapInfo* map_info = stack_maps_->Get(index);
  if (map_info == nullptr) {
    return 0;
  }
  return map_info->GetLoadBias(process_memory_);
}

std::string UnwindStackMap::GetFunctionName(uint64_t pc, uint64_t* offset) {
  *offset = 0;
  unwindstack::Maps* maps = stack_maps();

  // Get the map for this
  unwindstack::MapInfo* map_info = maps->Find(pc);
  if (map_info == nullptr || map_info->flags() & PROT_DEVICE_MAP) {
    return "";
  }

  if (arch_ == unwindstack::ARCH_UNKNOWN) {
    if (pid_ == getpid()) {
      arch_ = unwindstack::Regs::CurrentArch();
    } else {
      // Create a remote regs, to figure out the architecture.
      std::unique_ptr<unwindstack::Regs> regs(unwindstack::Regs::RemoteGet(pid_));
      arch_ = regs->Arch();
    }
  }

  unwindstack::Elf* elf = map_info->GetElf(process_memory(), arch_);

  unwindstack::SharedString name;
  uint64_t func_offset;
  if (!elf->GetFunctionName(elf->GetRelPc(pc, map_info), &name, &func_offset)) {
    return "";
  }
  *offset = func_offset;
  return name;
}

std::shared_ptr<unwindstack::Memory> UnwindStackMap::GetProcessMemory() {
  return process_memory_;
}

//-------------------------------------------------------------------------
// BacktraceMap create function.
//-------------------------------------------------------------------------
BacktraceMap* BacktraceMap::Create(pid_t pid, bool uncached) {
  BacktraceMap* map;

  if (uncached) {
    // Force use of the base class to parse the maps when this call is made.
    map = new BacktraceMap(pid);
  } else if (pid == getpid()) {
    map = new UnwindStackMap(0);
  } else {
    map = new UnwindStackMap(pid);
  }
  if (!map->Build()) {
    delete map;
    return nullptr;
  }
  return map;
}
