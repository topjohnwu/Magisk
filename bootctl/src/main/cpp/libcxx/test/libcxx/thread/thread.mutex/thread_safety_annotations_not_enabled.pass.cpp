//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-has-no-threads

// <mutex>

// This test does not define _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS so it
// should compile without any warnings or errors even though this pattern is not
// understood by the thread safety annotations.

#include <mutex>

int main() {
  std::mutex m;
  m.lock();
  {
    std::unique_lock<std::mutex> g(m, std::adopt_lock);
  }
}
