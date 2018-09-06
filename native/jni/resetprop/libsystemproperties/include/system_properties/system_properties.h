/*
 * Copyright (C) 2008 The Android Open Source Project
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

#pragma once

#include <stdint.h>
#include <sys/param.h>
#include <sys/system_properties.h>

#include "contexts.h"
#include "contexts_pre_split.h"
#include "contexts_serialized.h"
#include "contexts_split.h"

constexpr int PROP_FILENAME_MAX = 1024;

class SystemProperties {
 public:
  friend struct LocalPropertyTestState;
  friend class SystemPropertiesTest;
  // Note that system properties are initialized before libc calls static initializers, so
  // doing any initialization in this constructor is an error.  Even a Constructor that zero
  // initializes this class will clobber the previous property initialization.
  // We rely on the static SystemProperties in libc to be placed in .bss and zero initialized.
  SystemProperties() = default;
  // Special constructor for testing that also zero initializes the important members.
  explicit SystemProperties(bool initialized) : initialized_(initialized) {
  }

  DISALLOW_COPY_AND_ASSIGN(SystemProperties);

  bool Init(const char* filename);
  bool AreaInit(const char* filename, bool* fsetxattr_failed);
  uint32_t AreaSerial();
  const prop_info* Find(const char* name);
  int Read(const prop_info* pi, char* name, char* value);
  void ReadCallback(const prop_info* pi,
                    void (*callback)(void* cookie, const char* name, const char* value,
                                     uint32_t serial),
                    void* cookie);
  int Get(const char* name, char* value);
  int Update(prop_info* pi, const char* value, unsigned int len);
  int Add(const char* name, unsigned int namelen, const char* value, unsigned int valuelen);
  int Delete(const char *name);
  uint32_t Serial(const prop_info* pi);
  uint32_t WaitAny(uint32_t old_serial);
  bool Wait(const prop_info* pi, uint32_t old_serial, uint32_t* new_serial_ptr,
            const timespec* relative_timeout);
  const prop_info* FindNth(unsigned n);
  int Foreach(void (*propfn)(const prop_info* pi, void* cookie), void* cookie);

 private:
  // We don't want to use new or malloc in properties (b/31659220), and we don't want to waste a
  // full page by using mmap(), so we set aside enough space to create any context of the three
  // contexts.
  static constexpr size_t kMaxContextsAlign =
      MAX(alignof(ContextsSerialized), MAX(alignof(ContextsSplit), alignof(ContextsPreSplit)));
  static constexpr size_t kMaxContextsSize =
      MAX(sizeof(ContextsSerialized), MAX(sizeof(ContextsSplit), sizeof(ContextsPreSplit)));
  alignas(kMaxContextsAlign) char contexts_data_[kMaxContextsSize];
  Contexts* contexts_;

  bool initialized_;
  char property_filename_[PROP_FILENAME_MAX];
};
