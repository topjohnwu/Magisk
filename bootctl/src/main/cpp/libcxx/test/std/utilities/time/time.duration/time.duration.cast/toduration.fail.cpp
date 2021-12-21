//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// duration

// template <class ToDuration, class Rep, class Period>
//   ToDuration
//   duration_cast(const duration<Rep, Period>& d);

// ToDuration shall be an instantiation of duration.

#include <chrono>

int main()
{
    std::chrono::duration_cast<int>(std::chrono::milliseconds(3));
}
