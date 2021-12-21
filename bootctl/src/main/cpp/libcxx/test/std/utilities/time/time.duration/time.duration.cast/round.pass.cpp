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

// round

// template <class ToDuration, class Rep, class Period>
//   constexpr
//   ToDuration
//   ceil(const duration<Rep, Period>& d);

#include <chrono>
#include <type_traits>
#include <cassert>

template <class ToDuration, class FromDuration>
void
test(const FromDuration& f, const ToDuration& d)
{
    {
    typedef decltype(std::chrono::round<ToDuration>(f)) R;
    static_assert((std::is_same<R, ToDuration>::value), "");
    assert(std::chrono::round<ToDuration>(f) == d);
    }
}

int main()
{
//  7290000ms is 2 hours, 1 minute, and 30 seconds
    test(std::chrono::milliseconds( 7290000), std::chrono::hours( 2));
    test(std::chrono::milliseconds(-7290000), std::chrono::hours(-2));
    test(std::chrono::milliseconds( 7290000), std::chrono::minutes( 122));
    test(std::chrono::milliseconds(-7290000), std::chrono::minutes(-122));

    {
//  9000000ms is 2 hours and 30 minutes
    constexpr std::chrono::hours h1 = std::chrono::round<std::chrono::hours>(std::chrono::milliseconds(9000000));
    static_assert(h1.count() == 2, "");
    constexpr std::chrono::hours h2 = std::chrono::round<std::chrono::hours>(std::chrono::milliseconds(-9000000));
    static_assert(h2.count() == -2, "");
    }
}
