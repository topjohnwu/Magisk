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

#include "system_properties/prop_info.h"

#include <string.h>

constexpr static const char kLongLegacyError[] =
    "Must use __system_property_read_callback() to read";
static_assert(sizeof(kLongLegacyError) < prop_info::kLongLegacyErrorBufferSize,
              "Error message for long properties read by legacy libc must fit within 56 chars");

prop_info::prop_info(const char* name, uint32_t namelen, const char* value, uint32_t valuelen) {
  memcpy(this->name, name, namelen);
  this->name[namelen] = '\0';
  atomic_init(&this->serial, valuelen << 24);
  memcpy(this->value, value, valuelen);
  this->value[valuelen] = '\0';
}

prop_info::prop_info(const char* name, uint32_t namelen, uint32_t long_offset) {
  memcpy(this->name, name, namelen);
  this->name[namelen] = '\0';

  auto error_value_len = sizeof(kLongLegacyError) - 1;
  atomic_init(&this->serial, error_value_len << 24 | kLongFlag);
  memcpy(this->long_property.error_message, kLongLegacyError, sizeof(kLongLegacyError));

  this->long_property.offset = long_offset;
}
