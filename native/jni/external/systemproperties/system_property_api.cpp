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

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
//#include <sys/_system_properties.h>
#include <_system_properties.h>

#include <system_properties/prop_area.h>
#include <system_properties/system_properties.h>

#include "private/bionic_defs.h"

static SystemProperties system_properties;
static_assert(__is_trivially_constructible(SystemProperties),
              "System Properties must be trivially constructable");

// This is public because it was exposed in the NDK. As of 2017-01, ~60 apps reference this symbol.
// It is set to nullptr and never modified.
__BIONIC_WEAK_VARIABLE_FOR_NATIVE_BRIDGE
prop_area* __system_property_area__ = nullptr;

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_properties_init() {
  return system_properties.Init(PROP_FILENAME) ? 0 : -1;
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_set_filename(const char*) {
  return -1;
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_area_init() {
  bool fsetxattr_failed = false;
  return system_properties.AreaInit(PROP_FILENAME, &fsetxattr_failed) && !fsetxattr_failed ? 0 : -1;
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
uint32_t __system_property_area_serial() {
  return system_properties.AreaSerial();
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
const prop_info* __system_property_find(const char* name) {
  return system_properties.Find(name);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_read(const prop_info* pi, char* name, char* value) {
  return system_properties.Read(pi, name, value);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
void __system_property_read_callback(const prop_info* pi,
                                     void (*callback)(void* cookie, const char* name,
                                                      const char* value, uint32_t serial),
                                     void* cookie) {
  return system_properties.ReadCallback(pi, callback, cookie);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_get(const char* name, char* value) {
  return system_properties.Get(name, value);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_update(prop_info* pi, const char* value, unsigned int len) {
  return system_properties.Update(pi, value, len);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_add(const char* name, unsigned int namelen, const char* value,
                          unsigned int valuelen) {
  return system_properties.Add(name, namelen, value, valuelen);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_delete(const char* name) {
  return system_properties.Delete(name);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
uint32_t __system_property_serial(const prop_info* pi) {
  return system_properties.Serial(pi);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
uint32_t __system_property_wait_any(uint32_t old_serial) {
  return system_properties.WaitAny(old_serial);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
bool __system_property_wait(const prop_info* pi, uint32_t old_serial, uint32_t* new_serial_ptr,
                            const timespec* relative_timeout) {
  return system_properties.Wait(pi, old_serial, new_serial_ptr, relative_timeout);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
const prop_info* __system_property_find_nth(unsigned n) {
  return system_properties.FindNth(n);
}

__BIONIC_WEAK_FOR_NATIVE_BRIDGE
int __system_property_foreach(void (*propfn)(const prop_info* pi, void* cookie), void* cookie) {
  return system_properties.Foreach(propfn, cookie);
}
