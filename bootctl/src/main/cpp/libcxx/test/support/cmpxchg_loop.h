//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <atomic>

template <class A, class T>
bool cmpxchg_weak_loop(A& atomic, T& expected, T desired) {
  for (int i = 0; i < 10; i++) {
    if (atomic.compare_exchange_weak(expected, desired) == true) {
      return true;
    }
  }

  return false;
}

template <class A, class T>
bool cmpxchg_weak_loop(A& atomic, T& expected, T desired,
                       std::memory_order success,
                       std::memory_order failure) {
  for (int i = 0; i < 10; i++) {
    if (atomic.compare_exchange_weak(expected, desired, success,
                                     failure) == true) {
      return true;
    }
  }

  return false;
}

template <class A, class T>
bool c_cmpxchg_weak_loop(A* atomic, T* expected, T desired) {
  for (int i = 0; i < 10; i++) {
    if (std::atomic_compare_exchange_weak(atomic, expected, desired) == true) {
      return true;
    }
  }

  return false;
}

template <class A, class T>
bool c_cmpxchg_weak_loop(A* atomic, T* expected, T desired,
                         std::memory_order success,
                         std::memory_order failure) {
  for (int i = 0; i < 10; i++) {
    if (std::atomic_compare_exchange_weak_explicit(atomic, expected, desired,
                                                   success, failure) == true) {
      return true;
    }
  }

  return false;
}
