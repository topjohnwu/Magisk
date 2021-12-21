//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <chrono>

// abs

// template <class Rep, class Period>
//   constexpr duration<Rep, Period> abs(duration<Rep, Period> d)

#include <chrono>
#include <type_traits>
#include <cassert>

template <class Duration>
void
test(const Duration& f, const Duration& d)
{
    {
    typedef decltype(std::chrono::abs(f)) R;
    static_assert((std::is_same<R, Duration>::value), "");
    assert(std::chrono::abs(f) == d);
    }
}

int main()
{
//  7290000ms is 2 hours, 1 minute, and 30 seconds
    test(std::chrono::milliseconds( 7290000), std::chrono::milliseconds( 7290000));
    test(std::chrono::milliseconds(-7290000), std::chrono::milliseconds( 7290000));
    test(std::chrono::minutes( 122), std::chrono::minutes( 122));
    test(std::chrono::minutes(-122), std::chrono::minutes( 122));
    test(std::chrono::hours(0), std::chrono::hours(0));

    {
//  9000000ms is 2 hours and 30 minutes
    constexpr std::chrono::hours h1 = std::chrono::abs(std::chrono::hours(-3));
    static_assert(h1.count() == 3, "");
    constexpr std::chrono::hours h2 = std::chrono::abs(std::chrono::hours(3));
    static_assert(h2.count() == 3, "");
    }
}
