//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <filesystem>

// typedef TrivialClock file_time_type;

#include "filesystem_include.hpp"
#include <chrono>
#include <type_traits>

#include "test_macros.h"

// system_clock is used because it meets the requirements of TrivialClock,
// and the resolution and range of system_clock should match the operating
// system's file time type.

void test_trivial_clock() {
  using namespace fs;
  using Clock = file_time_type::clock;
  ASSERT_NOEXCEPT(Clock::now());
  ASSERT_SAME_TYPE(decltype(Clock::now()), file_time_type);
  ASSERT_SAME_TYPE(Clock::time_point, file_time_type);
  volatile auto* odr_use = &Clock::is_steady;
  ((void)odr_use);
}

void test_time_point_resolution_and_range() {
  using namespace fs;
  using Dur = file_time_type::duration;
  using Period = Dur::period;
  ASSERT_SAME_TYPE(Period, std::nano);
}

int main() {
  test_trivial_clock();
  test_time_point_resolution_and_range();
}
