//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14
// <chrono>

// floor

// template <class ToDuration, class Rep, class Period>
//   ToDuration
//   floor(const duration<Rep, Period>& d);

// ToDuration shall be an instantiation of duration.

#include <chrono>

int main()
{
    std::chrono::floor<int>(std::chrono::milliseconds(3));
}
