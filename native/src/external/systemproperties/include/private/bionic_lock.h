/*
 * Copyright (C) 2015 The Android Open Source Project
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
#include "private/bionic_futex.h"
#include "private/bionic_macros.h"

// Lock is used in places like pthread_rwlock_t, which can be initialized without calling
// an initialization function. So make sure Lock can be initialized by setting its memory to 0.
class Lock {
 private:
  enum LockState {
    Unlocked = 0,
    LockedWithoutWaiter,
    LockedWithWaiter,
  };
  _Atomic(LockState) state;
  bool process_shared;

 public:
  void init(bool process_shared) {
    atomic_init(&state, Unlocked);
    this->process_shared = process_shared;
  }

  bool trylock() {
    LockState old_state = Unlocked;
    return __predict_true(atomic_compare_exchange_strong_explicit(&state, &old_state,
                        LockedWithoutWaiter, memory_order_acquire, memory_order_relaxed));
  }

  void lock() {
    LockState old_state = Unlocked;
    if (__predict_true(atomic_compare_exchange_strong_explicit(&state, &old_state,
                         LockedWithoutWaiter, memory_order_acquire, memory_order_relaxed))) {
      return;
    }
    while (atomic_exchange_explicit(&state, LockedWithWaiter, memory_order_acquire) != Unlocked) {
      // TODO: As the critical section is brief, it is a better choice to spin a few times befor sleeping.
      __futex_wait_ex(&state, process_shared, LockedWithWaiter);
    }
    return;
  }

  void unlock() {
    bool shared = process_shared; /* cache to local variable */
    if (atomic_exchange_explicit(&state, Unlocked, memory_order_release) == LockedWithWaiter) {
      __futex_wake_ex(&state, shared, 1);
    }
  }
};

class LockGuard {
 public:
  explicit LockGuard(Lock& lock) : lock_(lock) {
    lock_.lock();
  }
  ~LockGuard() {
    lock_.unlock();
  }

  BIONIC_DISALLOW_COPY_AND_ASSIGN(LockGuard);

 private:
  Lock& lock_;
};
