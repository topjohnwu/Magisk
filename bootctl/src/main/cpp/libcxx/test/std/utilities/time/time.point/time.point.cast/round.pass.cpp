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

// template <class ToDuration, class Clock, class Duration>
//   time_point<Clock, ToDuration>
//   round(const time_point<Clock, Duration>& t);

#include <chrono>
#include <type_traits>
#include <cassert>

template <class FromDuration, class ToDuration>
void
test(const FromDuration& df, const ToDuration& d)
{
    typedef std::chrono::system_clock Clock;
    typedef std::chrono::time_point<Clock, FromDuration> FromTimePoint;
    typedef std::chrono::time_point<Clock, ToDuration> ToTimePoint;
    {
    FromTimePoint f(df);
    ToTimePoint t(d);
    typedef decltype(std::chrono::round<ToDuration>(f)) R;
    static_assert((std::is_same<R, ToTimePoint>::value), "");
    assert(std::chrono::round<ToDuration>(f) == t);
    }
}

template<class FromDuration, long long From, class ToDuration, long long To>
void test_constexpr ()
{
    typedef std::chrono::system_clock Clock;
    typedef std::chrono::time_point<Clock, FromDuration> FromTimePoint;
    typedef std::chrono::time_point<Clock, ToDuration> ToTimePoint;
    {
    constexpr FromTimePoint f{FromDuration{From}};
    constexpr ToTimePoint   t{ToDuration{To}};
    static_assert(std::chrono::round<ToDuration>(f) == t, "");
    }
}

int main()
{
//  7290000ms is 2 hours, 1 minute, and 30 seconds
    test(std::chrono::milliseconds( 7290000), std::chrono::hours( 2));
    test(std::chrono::milliseconds(-7290000), std::chrono::hours(-2));
    test(std::chrono::milliseconds( 7290000), std::chrono::minutes( 122));
    test(std::chrono::milliseconds(-7290000), std::chrono::minutes(-122));

//  9000000ms is 2 hours and 30 minutes
    test_constexpr<std::chrono::milliseconds, 9000000, std::chrono::hours,    2> ();
    test_constexpr<std::chrono::milliseconds,-9000000, std::chrono::hours,   -2> ();
    test_constexpr<std::chrono::milliseconds, 9000001, std::chrono::minutes, 150> ();
    test_constexpr<std::chrono::milliseconds,-9000001, std::chrono::minutes,-150> ();

    test_constexpr<std::chrono::milliseconds, 9000000, std::chrono::seconds, 9000> ();
    test_constexpr<std::chrono::milliseconds,-9000000, std::chrono::seconds,-9000> ();
}
