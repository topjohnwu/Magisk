//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// time_point

// Test default template arg:

// template <class Clock, class Duration = typename Clock::duration>
//   class time_point;

#include <chrono>
#include <type_traits>

int main()
{
    static_assert((std::is_same<std::chrono::system_clock::duration,
                   std::chrono::time_point<std::chrono::system_clock>::duration>::value), "");
}
