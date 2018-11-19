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
#ifndef _BIONIC_FUTEX_H
#define _BIONIC_FUTEX_H

#include <errno.h>
#include <linux/futex.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/syscall.h>
#include <unistd.h>

struct timespec;

static inline __always_inline int __futex(volatile void* ftx, int op, int value,
                                          const timespec* timeout, int bitset) {
  // Our generated syscall assembler sets errno, but our callers (pthread functions) don't want to.
  int saved_errno = errno;
  int result = syscall(__NR_futex, ftx, op, value, timeout, NULL, bitset);
  if (__predict_false(result == -1)) {
    result = -errno;
    errno = saved_errno;
  }
  return result;
}

static inline int __futex_wake(volatile void* ftx, int count) {
  return __futex(ftx, FUTEX_WAKE, count, nullptr, 0);
}

static inline int __futex_wake_ex(volatile void* ftx, bool shared, int count) {
  return __futex(ftx, shared ? FUTEX_WAKE : FUTEX_WAKE_PRIVATE, count, nullptr, 0);
}

static inline int __futex_wait(volatile void* ftx, int value, const timespec* timeout) {
  return __futex(ftx, FUTEX_WAIT, value, timeout, 0);
}

static inline int __futex_wait_ex(volatile void* ftx, bool shared, int value) {
  return __futex(ftx, (shared ? FUTEX_WAIT_BITSET : FUTEX_WAIT_BITSET_PRIVATE), value, nullptr,
                 FUTEX_BITSET_MATCH_ANY);
}

__LIBC_HIDDEN__ int __futex_wait_ex(volatile void* ftx, bool shared, int value,
                                    bool use_realtime_clock, const timespec* abs_timeout);

static inline int __futex_pi_unlock(volatile void* ftx, bool shared) {
  return __futex(ftx, shared ? FUTEX_UNLOCK_PI : FUTEX_UNLOCK_PI_PRIVATE, 0, nullptr, 0);
}

__LIBC_HIDDEN__ int __futex_pi_lock_ex(volatile void* ftx, bool shared, bool use_realtime_clock,
                                       const timespec* abs_timeout);

#endif /* _BIONIC_FUTEX_H */
