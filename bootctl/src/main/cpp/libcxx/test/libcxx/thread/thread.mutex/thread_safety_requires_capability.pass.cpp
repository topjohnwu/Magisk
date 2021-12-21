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

std::mutex m;
int foo __attribute__((guarded_by(m)));

void increment() __attribute__((requires_capability(m))) {
  foo++;
}

int main() {
  m.lock();
  increment();
  m.unlock();
}
