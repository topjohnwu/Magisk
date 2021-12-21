//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <chrono>

// file_time

#include <chrono>

#include "test_macros.h"

template <class Dur>
void test() {
  ASSERT_SAME_TYPE(std::chrono::file_time<Dur>, std::chrono::time_point<std::chrono::file_clock, Dur>);
}

int main() {
  test<std::chrono::nanoseconds>();
  test<std::chrono::minutes>();
  test<std::chrono::hours>();
}