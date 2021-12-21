//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// On Windows Clang bugs out when both __declspec and __attribute__ are present,
// the processing goes awry preventing the definition of the types.
// XFAIL: LIBCXX-WINDOWS-FIXME

// UNSUPPORTED: libcpp-has-no-threads
// REQUIRES: thread-safety

// <mutex>

// MODULES_DEFINES: _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS
#define _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS

#include <mutex>

#include "test_macros.h"

std::mutex m;
int foo __attribute__((guarded_by(m)));

static void scoped() {
#if TEST_STD_VER >= 17
  std::scoped_lock<std::mutex> lock(m);
  foo++;
#endif
}

int main() {
  scoped();
  std::lock_guard<std::mutex> lock(m);
  foo++;
}
