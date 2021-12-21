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

// template <class ToDuration, class Clock, class Duration>
//   time_point<Clock, ToDuration>
//   time_point_cast(const time_point<Clock, Duration>& t);

// ToDuration shall be an instantiation of duration.

#include <chrono>

int main()
{
    typedef std::chrono::system_clock Clock;
    typedef std::chrono::time_point<Clock, std::chrono::milliseconds> FromTimePoint;
    typedef std::chrono::time_point<Clock, std::chrono::minutes> ToTimePoint;
    std::chrono::time_point_cast<ToTimePoint>(FromTimePoint(std::chrono::milliseconds(3)));
}
