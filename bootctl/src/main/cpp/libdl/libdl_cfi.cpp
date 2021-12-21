/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <sys/mman.h>

#include "private/CFIShadow.h"

__attribute__((__weak__, visibility("default"))) extern "C" void __loader_cfi_fail(
    uint64_t CallSiteTypeId, void* Ptr, void* DiagData, void* CallerPc);

// Base address of the CFI shadow. Passed down from the linker in __cfi_init()
// and does not change after that. The contents of the shadow change in
// dlopen/dlclose.
static struct {
  uintptr_t v;
  char padding[PAGE_SIZE - sizeof(v)];
} shadow_base_storage alignas(PAGE_SIZE);

// __cfi_init is called by the loader as soon as the shadow is mapped. This may happen very early
// during startup, before libdl.so global constructors, and, on i386, even before __libc_sysinfo is
// initialized. This function should not do any system calls.
extern "C" uintptr_t* __cfi_init(uintptr_t shadow_base) {
  shadow_base_storage.v = shadow_base;
  static_assert(sizeof(shadow_base_storage) == PAGE_SIZE, "");
  return &shadow_base_storage.v;
}

// Returns the size of the CFI shadow mapping, or 0 if CFI is not (yet) used in this process.
extern "C" size_t __cfi_shadow_size() {
  return shadow_base_storage.v != 0 ? CFIShadow::kShadowSize : 0;
}

static uint16_t shadow_load(void* p) {
  // Untag the pointer to move it into the address space covered by the shadow.
  uintptr_t addr = reinterpret_cast<uintptr_t>(untag_address(p));
  uintptr_t ofs = CFIShadow::MemToShadowOffset(addr);
  if (ofs > CFIShadow::kShadowSize) return CFIShadow::kInvalidShadow;
  return *reinterpret_cast<uint16_t*>(shadow_base_storage.v + ofs);
}

static uintptr_t cfi_check_addr(uint16_t v, void* Ptr) {
  uintptr_t addr = reinterpret_cast<uintptr_t>(Ptr);
  // The aligned range of [0, kShadowAlign) uses a single shadow element, therefore all pointers in
  // this range must get the same aligned_addr below. This matches CFIShadowWriter::Add; not the
  // same as align_up().
  uintptr_t aligned_addr = align_down(addr, CFIShadow::kShadowAlign) + CFIShadow::kShadowAlign;
  uintptr_t p = aligned_addr - (static_cast<uintptr_t>(v - CFIShadow::kRegularShadowMin)
                                << CFIShadow::kCfiCheckGranularity);
#ifdef __arm__
  // Assume Thumb encoding. FIXME: force thumb at compile time?
  p++;
#endif
  return p;
}

static inline void cfi_slowpath_common(uint64_t CallSiteTypeId, void* Ptr, void* DiagData) {
  uint16_t v = shadow_load(Ptr);
  switch (v) {
    case CFIShadow::kInvalidShadow:
      __loader_cfi_fail(CallSiteTypeId, Ptr, DiagData, __builtin_return_address(0));
      break;
    case CFIShadow::kUncheckedShadow:
      break;
    default:
      reinterpret_cast<CFIShadow::CFICheckFn>(cfi_check_addr(v, Ptr))(CallSiteTypeId, Ptr, DiagData);
  }
}

extern "C" void __cfi_slowpath(uint64_t CallSiteTypeId, void* Ptr) {
  cfi_slowpath_common(CallSiteTypeId, Ptr, nullptr);
}

extern "C" void __cfi_slowpath_diag(uint64_t CallSiteTypeId, void* Ptr, void* DiagData) {
  cfi_slowpath_common(CallSiteTypeId, Ptr, DiagData);
}
