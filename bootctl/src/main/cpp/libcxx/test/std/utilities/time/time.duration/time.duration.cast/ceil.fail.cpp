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

// ceil

// template <class ToDuration, class Rep, class Period>
//   ToDuration
//   ceil(const duration<Rep, Period>& d);

// ToDuration shall be an instantiation of duration.

#include <chrono>

int main()
{
    std::chrono::ceil<int>(std::chrono::milliseconds(3));
}
