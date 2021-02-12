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

#include "system_properties/system_properties.h"

#include <errno.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <new>

#include <async_safe/log.h>

#include "private/ErrnoRestorer.h"
#include "private/bionic_futex.h"

#include "system_properties/context_node.h"
#include "system_properties/prop_area.h"
#include "system_properties/prop_info.h"

#define SERIAL_DIRTY(serial) ((serial)&1)
#define SERIAL_VALUE_LEN(serial) ((serial) >> 24)

static bool is_dir(const char* pathname) {
  struct stat info;
  if (stat(pathname, &info) == -1) {
    return false;
  }
  return S_ISDIR(info.st_mode);
}

bool SystemProperties::Init(const char* filename) {
  // This is called from __libc_init_common, and should leave errno at 0 (http://b/37248982).
  ErrnoRestorer errno_restorer;

  if (initialized_) {
    /* resetprop remove */
    // contexts_->ResetAccess();
    return true;
  }

  if (strlen(filename) >= PROP_FILENAME_MAX) {
    return false;
  }
  strcpy(property_filename_, filename);

  if (is_dir(property_filename_)) {
    if (access("/dev/__properties__/property_info", R_OK) == 0) {
      contexts_ = new (contexts_data_) ContextsSerialized();
      if (!contexts_->Initialize(false, property_filename_, nullptr)) {
        return false;
      }
    } else {
      contexts_ = new (contexts_data_) ContextsSplit();
      if (!contexts_->Initialize(false, property_filename_, nullptr)) {
        return false;
      }
    }
  } else {
    contexts_ = new (contexts_data_) ContextsPreSplit();
    if (!contexts_->Initialize(false, property_filename_, nullptr)) {
      return false;
    }
  }
  initialized_ = true;
  return true;
}

bool SystemProperties::AreaInit(const char* filename, bool* fsetxattr_failed) {
  if (strlen(filename) >= PROP_FILENAME_MAX) {
    return false;
  }
  strcpy(property_filename_, filename);

  contexts_ = new (contexts_data_) ContextsSerialized();
  if (!contexts_->Initialize(true, property_filename_, fsetxattr_failed)) {
    return false;
  }
  initialized_ = true;
  return true;
}

uint32_t SystemProperties::AreaSerial() {
  if (!initialized_) {
    return -1;
  }

  prop_area* pa = contexts_->GetSerialPropArea();
  if (!pa) {
    return -1;
  }

  // Make sure this read fulfilled before __system_property_serial
  return atomic_load_explicit(pa->serial(), memory_order_acquire);
}

const prop_info* SystemProperties::Find(const char* name) {
  if (!initialized_) {
    return nullptr;
  }

  prop_area* pa = contexts_->GetPropAreaForName(name);
  if (!pa) {
    async_safe_format_log(ANDROID_LOG_ERROR, "libc", "Access denied finding property \"%s\"", name);
    return nullptr;
  }

  return pa->find(name);
}

static bool is_read_only(const char* name) {
  return strncmp(name, "ro.", 3) == 0;
}

int SystemProperties::Read(const prop_info* pi, char* name, char* value) {
  while (true) {
    uint32_t serial = Serial(pi);  // acquire semantics
    size_t len = SERIAL_VALUE_LEN(serial);
    memcpy(value, pi->value, len + 1);
    // TODO: Fix the synchronization scheme here.
    // There is no fully supported way to implement this kind
    // of synchronization in C++11, since the memcpy races with
    // updates to pi, and the data being accessed is not atomic.
    // The following fence is unintuitive, but would be the
    // correct one if memcpy used memory_order_relaxed atomic accesses.
    // In practice it seems unlikely that the generated code would
    // would be any different, so this should be OK.
    atomic_thread_fence(memory_order_acquire);
    if (serial == load_const_atomic(&(pi->serial), memory_order_relaxed)) {
      if (name != nullptr) {
        size_t namelen = strlcpy(name, pi->name, PROP_NAME_MAX);
        if (namelen >= PROP_NAME_MAX) {
          async_safe_format_log(ANDROID_LOG_ERROR, "libc",
                                "The property name length for \"%s\" is >= %d;"
                                " please use __system_property_read_callback"
                                " to read this property. (the name is truncated to \"%s\")",
                                pi->name, PROP_NAME_MAX - 1, name);
        }
      }
      if (is_read_only(pi->name) && pi->is_long()) {
        async_safe_format_log(
            ANDROID_LOG_ERROR, "libc",
            "The property \"%s\" has a value with length %zu that is too large for"
            " __system_property_get()/__system_property_read(); use"
            " __system_property_read_callback() instead.",
            pi->name, strlen(pi->long_value()));
      }
      return len;
    }
  }
}

void SystemProperties::ReadCallback(const prop_info* pi,
                                    void (*callback)(void* cookie, const char* name,
                                                     const char* value, uint32_t serial),
                                    void* cookie) {
  // Read only properties don't need to copy the value to a temporary buffer, since it can never
  // change.
  if (is_read_only(pi->name)) {
    uint32_t serial = Serial(pi);
    if (pi->is_long()) {
      callback(cookie, pi->name, pi->long_value(), serial);
    } else {
      callback(cookie, pi->name, pi->value, serial);
    }
    return;
  }

  while (true) {
    uint32_t serial = Serial(pi);  // acquire semantics
    size_t len = SERIAL_VALUE_LEN(serial);
    char value_buf[len + 1];

    memcpy(value_buf, pi->value, len);
    value_buf[len] = '\0';

    // TODO: see todo in Read function
    atomic_thread_fence(memory_order_acquire);
    if (serial == load_const_atomic(&(pi->serial), memory_order_relaxed)) {
      callback(cookie, pi->name, value_buf, serial);
      return;
    }
  }
}

int SystemProperties::Get(const char* name, char* value) {
  const prop_info* pi = Find(name);

  if (pi != nullptr) {
    return Read(pi, nullptr, value);
  } else {
    value[0] = 0;
    return 0;
  }
}

int SystemProperties::Update(prop_info* pi, const char* value, unsigned int len) {
  if (len >= PROP_VALUE_MAX) {
    return -1;
  }

  if (!initialized_) {
    return -1;
  }

  prop_area* pa = contexts_->GetSerialPropArea();
  if (!pa) {
    return -1;
  }

  uint32_t serial = atomic_load_explicit(&pi->serial, memory_order_relaxed);
  serial |= 1;
  atomic_store_explicit(&pi->serial, serial, memory_order_relaxed);
  // The memcpy call here also races.  Again pretend it
  // used memory_order_relaxed atomics, and use the analogous
  // counterintuitive fence.
  atomic_thread_fence(memory_order_release);
  strlcpy(pi->value, value, len + 1);

  atomic_store_explicit(&pi->serial, (len << 24) | ((serial + 1) & 0xffffff), memory_order_release);
  __futex_wake(&pi->serial, INT32_MAX);

  atomic_store_explicit(pa->serial(), atomic_load_explicit(pa->serial(), memory_order_relaxed) + 1,
                        memory_order_release);
  __futex_wake(pa->serial(), INT32_MAX);

  return 0;
}

int SystemProperties::Add(const char* name, unsigned int namelen, const char* value,
                          unsigned int valuelen) {
  if (valuelen >= PROP_VALUE_MAX && !is_read_only(name)) {
    return -1;
  }

  if (namelen < 1) {
    return -1;
  }

  if (!initialized_) {
    return -1;
  }

  prop_area* serial_pa = contexts_->GetSerialPropArea();
  if (serial_pa == nullptr) {
    return -1;
  }

  prop_area* pa = contexts_->GetPropAreaForName(name);
  if (!pa) {
    async_safe_format_log(ANDROID_LOG_ERROR, "libc", "Access denied adding property \"%s\"", name);
    return -1;
  }

  bool ret = pa->add(name, namelen, value, valuelen);
  if (!ret) {
    return -1;
  }

  // There is only a single mutator, but we want to make sure that
  // updates are visible to a reader waiting for the update.
  atomic_store_explicit(serial_pa->serial(),
                        atomic_load_explicit(serial_pa->serial(), memory_order_relaxed) + 1,
                        memory_order_release);
  __futex_wake(serial_pa->serial(), INT32_MAX);
  return 0;
}

int SystemProperties::Delete(const char *name) {
  if (!initialized_) {
    return -1;
  }

  prop_area* serial_pa = contexts_->GetSerialPropArea();
  if (serial_pa == nullptr) {
    return -1;
  }

  prop_area* pa = contexts_->GetPropAreaForName(name);
  if (!pa) {
    async_safe_format_log(ANDROID_LOG_ERROR, "libc", "Access denied adding property \"%s\"", name);
    return -1;
  }

  bool ret = pa->del(name);
  if (!ret) {
    return -1;
  }

  // There is only a single mutator, but we want to make sure that
  // updates are visible to a reader waiting for the update.
  atomic_store_explicit(serial_pa->serial(),
                        atomic_load_explicit(serial_pa->serial(), memory_order_relaxed) + 1,
                        memory_order_release);
  __futex_wake(serial_pa->serial(), INT32_MAX);
  return 0;
}

// Wait for non-locked serial, and retrieve it with acquire semantics.
uint32_t SystemProperties::Serial(const prop_info* pi) {
  uint32_t serial = load_const_atomic(&pi->serial, memory_order_acquire);
  while (SERIAL_DIRTY(serial)) {
    __futex_wait(const_cast<_Atomic(uint_least32_t)*>(&pi->serial), serial, nullptr);
    serial = load_const_atomic(&pi->serial, memory_order_acquire);
  }
  return serial;
}

uint32_t SystemProperties::WaitAny(uint32_t old_serial) {
  uint32_t new_serial;
  Wait(nullptr, old_serial, &new_serial, nullptr);
  return new_serial;
}

bool SystemProperties::Wait(const prop_info* pi, uint32_t old_serial, uint32_t* new_serial_ptr,
                            const timespec* relative_timeout) {
  // Are we waiting on the global serial or a specific serial?
  atomic_uint_least32_t* serial_ptr;
  if (pi == nullptr) {
    if (!initialized_) {
      return -1;
    }

    prop_area* serial_pa = contexts_->GetSerialPropArea();
    if (serial_pa == nullptr) {
      return -1;
    }

    serial_ptr = serial_pa->serial();
  } else {
    serial_ptr = const_cast<atomic_uint_least32_t*>(&pi->serial);
  }

  uint32_t new_serial;
  do {
    int rc;
    if ((rc = __futex_wait(serial_ptr, old_serial, relative_timeout)) != 0 && rc == -ETIMEDOUT) {
      return false;
    }
    new_serial = load_const_atomic(serial_ptr, memory_order_acquire);
  } while (new_serial == old_serial);

  *new_serial_ptr = new_serial;
  return true;
}

const prop_info* SystemProperties::FindNth(unsigned n) {
  struct find_nth {
    const uint32_t sought;
    uint32_t current;
    const prop_info* result;

    explicit find_nth(uint32_t n) : sought(n), current(0), result(nullptr) {
    }
    static void fn(const prop_info* pi, void* ptr) {
      find_nth* self = reinterpret_cast<find_nth*>(ptr);
      if (self->current++ == self->sought) self->result = pi;
    }
  } state(n);
  Foreach(find_nth::fn, &state);
  return state.result;
}

int SystemProperties::Foreach(void (*propfn)(const prop_info* pi, void* cookie), void* cookie) {
  if (!initialized_) {
    return -1;
  }

  contexts_->ForEach(propfn, cookie);

  return 0;
}
