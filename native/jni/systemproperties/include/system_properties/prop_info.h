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

#include <stdatomic.h>
#include <stdint.h>
//#include <sys/system_properties.h>
#include "../system_properties.h"

#include "private/bionic_macros.h"

// The C11 standard doesn't allow atomic loads from const fields,
// though C++11 does.  Fudge it until standards get straightened out.
static inline uint_least32_t load_const_atomic(const atomic_uint_least32_t* s, memory_order mo) {
  atomic_uint_least32_t* non_const_s = const_cast<atomic_uint_least32_t*>(s);
  return atomic_load_explicit(non_const_s, mo);
}

struct prop_info {
  // Read only properties will not set anything but the bottom most bit of serial and the top byte.
  // We borrow the 2nd from the top byte for extra flags, and use the bottom most bit of that for
  // our first user, kLongFlag.
  constexpr static uint32_t kLongFlag = 1 << 16;

  // The error message fits in part of a union with the previous 92 char property value so there
  // must be room left over after the error message for the offset to the new longer property value
  // and future expansion fields if needed. Note that this value cannot ever increase.  The offset
  // to the new longer property value appears immediately after it, so an increase of this size will
  // break compatibility.
  constexpr static size_t kLongLegacyErrorBufferSize = 56;

 public:
  atomic_uint_least32_t serial;
  // we need to keep this buffer around because the property
  // value can be modified whereas name is constant.
  union {
    char value[PROP_VALUE_MAX];
    struct {
      char error_message[kLongLegacyErrorBufferSize];
      uint32_t offset;
    } long_property;
  };
  char name[0];

  bool is_long() const {
    return (load_const_atomic(&serial, memory_order_relaxed) & kLongFlag) != 0;
  }

  const char* long_value() const {
    // We can't store pointers here since this is shared memory that will have different absolute
    // pointers in different processes.  We don't have data_ from prop_area, but since we know
    // `this` is data_ + some offset and long_value is data_ + some other offset, we calculate the
    // offset from `this` to long_value and store it as long_property.offset.
    return reinterpret_cast<const char*>(this) + long_property.offset;
  }

  prop_info(const char* name, uint32_t namelen, const char* value, uint32_t valuelen);
  prop_info(const char* name, uint32_t namelen, uint32_t long_offset);

 private:
  BIONIC_DISALLOW_IMPLICIT_CONSTRUCTORS(prop_info);
};

static_assert(sizeof(prop_info) == 96, "sizeof struct prop_info must be 96 bytes");
