/*
 * Copyright (C) 2017 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "system_properties/contexts_serialized.h"

#include <fcntl.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <new>

#include <async_safe/log.h>

#include "system_properties/system_properties.h"

bool ContextsSerialized::InitializeContextNodes() {
  auto num_context_nodes = property_info_area_file_->num_contexts();
  auto context_nodes_mmap_size = sizeof(ContextNode) * num_context_nodes;
  // We want to avoid malloc in system properties, so we take an anonymous map instead (b/31659220).
  void* const map_result = mmap(nullptr, context_nodes_mmap_size, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (map_result == MAP_FAILED) {
    return false;
  }

  prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, map_result, context_nodes_mmap_size,
        "System property context nodes");

  context_nodes_ = reinterpret_cast<ContextNode*>(map_result);
  num_context_nodes_ = num_context_nodes;
  context_nodes_mmap_size_ = context_nodes_mmap_size;

  for (size_t i = 0; i < num_context_nodes; ++i) {
    new (&context_nodes_[i]) ContextNode(property_info_area_file_->context(i), filename_);
  }

  return true;
}

bool ContextsSerialized::MapSerialPropertyArea(bool access_rw, bool* fsetxattr_failed) {
  char filename[PROP_FILENAME_MAX];
  int len = async_safe_format_buffer(filename, sizeof(filename), "%s/properties_serial", filename_);
  if (len < 0 || len >= PROP_FILENAME_MAX) {
    serial_prop_area_ = nullptr;
    return false;
  }

  if (access_rw) {
    serial_prop_area_ =
        prop_area::map_prop_area_rw(filename, "u:object_r:properties_serial:s0", fsetxattr_failed);
  } else {
    serial_prop_area_ = prop_area::map_prop_area(filename);
  }
  return serial_prop_area_;
}

bool ContextsSerialized::InitializeProperties() {
  if (!property_info_area_file_.LoadDefaultPath()) {
    return false;
  }

  if (!InitializeContextNodes()) {
    FreeAndUnmap();
    return false;
  }

  return true;
}

bool ContextsSerialized::Initialize(bool writable, const char* filename, bool* fsetxattr_failed) {
  filename_ = filename;
  if (!InitializeProperties()) {
    return false;
  }

  if (writable) {
    mkdir(filename_, S_IRWXU | S_IXGRP | S_IXOTH);
    bool open_failed = false;
    if (fsetxattr_failed) {
      *fsetxattr_failed = false;
    }

    for (size_t i = 0; i < num_context_nodes_; ++i) {
      if (!context_nodes_[i].Open(true, fsetxattr_failed)) {
        open_failed = true;
      }
    }
    if (open_failed || !MapSerialPropertyArea(true, fsetxattr_failed)) {
      FreeAndUnmap();
      return false;
    }
  } else {
    if (!MapSerialPropertyArea(false, nullptr)) {
      FreeAndUnmap();
      return false;
    }
  }
  return true;
}

prop_area* ContextsSerialized::GetPropAreaForName(const char* name) {
  uint32_t index;
  property_info_area_file_->GetPropertyInfoIndexes(name, &index, nullptr);
  if (index == ~0u || index >= num_context_nodes_) {
    async_safe_format_log(ANDROID_LOG_ERROR, "libc", "Could not find context for property \"%s\"",
                          name);
    return nullptr;
  }
  auto* context_node = &context_nodes_[index];
  if (!context_node->pa()) {
    // We explicitly do not check no_access_ in this case because unlike the
    // case of foreach(), we want to generate an selinux audit for each
    // non-permitted property access in this function.
    context_node->Open(false, nullptr);
  }
  return context_node->pa();
}

void ContextsSerialized::ForEach(void (*propfn)(const prop_info* pi, void* cookie), void* cookie) {
  for (size_t i = 0; i < num_context_nodes_; ++i) {
    if (context_nodes_[i].CheckAccessAndOpen()) {
      context_nodes_[i].pa()->foreach (propfn, cookie);
    }
  }
}

void ContextsSerialized::ResetAccess() {
  for (size_t i = 0; i < num_context_nodes_; ++i) {
    context_nodes_[i].ResetAccess();
  }
}

void ContextsSerialized::FreeAndUnmap() {
  property_info_area_file_.Reset();
  if (context_nodes_ != nullptr) {
    for (size_t i = 0; i < num_context_nodes_; ++i) {
      context_nodes_[i].Unmap();
    }
    munmap(context_nodes_, context_nodes_mmap_size_);
    context_nodes_ = nullptr;
  }
  prop_area::unmap_prop_area(&serial_prop_area_);
  serial_prop_area_ = nullptr;
}
