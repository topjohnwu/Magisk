// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that entities declared [[nodiscard]] as at extension by libc++, are
// only actually declared such when _LIBCPP_ENABLE_NODISCARD is specified.

// This test intentionally leaks memory, so it is unsupported under ASAN.
// UNSUPPORTED: asan

// All entities to which libc++ applies [[nodiscard]] as an extension should
// be tested here and in nodiscard_extensions.fail.cpp. They should also
// be listed in `UsingLibcxx.rst` in the documentation for the extension.

#include <memory>

#include "test_macros.h"

int main() {
  {
    std::get_temporary_buffer<int>(1); // intentional memory leak.
  }
}
