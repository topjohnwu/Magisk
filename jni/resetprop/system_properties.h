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

#ifndef _INCLUDE_SYS_SYSTEM_PROPERTIES_H
#define _INCLUDE_SYS_SYSTEM_PROPERTIES_H

#include <sys/cdefs.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

__BEGIN_DECLS

typedef struct prop_info prop_info;

#define PROP_VALUE_MAX  92

/*
 * Sets system property `key` to `value`, creating the system property if it doesn't already exist.
 */
int __system_property_set(const char* key, const char* value) __INTRODUCED_IN(12);

/*
 * Returns a `prop_info` corresponding system property `name`, or nullptr if it doesn't exist.
 * Use __system_property_read_callback to query the current value.
 *
 * Property lookup is expensive, so it can be useful to cache the result of this function.
 */
const prop_info* __system_property_find(const char* name);

/*
 * Calls `callback` with a consistent trio of name, value, and serial number for property `pi`.
 */
void __system_property_read_callback(const prop_info *pi,
    void (*callback)(void* cookie, const char *name, const char *value, uint32_t serial),
    void* cookie) __INTRODUCED_IN_FUTURE;

/*
 * Passes a `prop_info` for each system property to the provided
 * callback.  Use __system_property_read_callback() to read the value.
 *
 * This method is for inspecting and debugging the property system, and not generally useful.
 */
int __system_property_foreach(void (*propfn)(const prop_info* pi, void* cookie), void* cookie)
  __INTRODUCED_IN(19);

/*
 * Waits for the specific system property identified by `pi` to be updated
 * past `old_serial`. Waits no longer than `relative_timeout`, or forever
 * if `relaive_timeout` is null.
 *
 * If `pi` is null, waits for the global serial number instead.
 *
 * If you don't know the current serial, use 0.
 *
 * Returns true and updates `*new_serial_ptr` on success, or false if the call
 * timed out.
 */
struct timespec;
bool __system_property_wait(const prop_info* pi,
                            uint32_t old_serial,
                            uint32_t* new_serial_ptr,
                            const struct timespec* relative_timeout)
    __INTRODUCED_IN_FUTURE;

/* Deprecated. In Android O and above, there's no limit on property name length. */
#define PROP_NAME_MAX   32
/* Deprecated. Use __system_property_read_callback instead. */
int __system_property_read(const prop_info *pi, char *name, char *value);
/* Deprecated. Use __system_property_read_callback instead. */
int __system_property_get(const char *name, char *value);
/* Deprecated. Use __system_property_foreach instead. Aborts in Android O and above. */
const prop_info *__system_property_find_nth(unsigned n) __REMOVED_IN(26);

__END_DECLS

#endif
