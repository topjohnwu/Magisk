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

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

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
    typedef decltype(std::chrono::time_point_cast<ToDuration>(f)) R;
    static_assert((std::is_same<R, ToTimePoint>::value), "");
    assert(std::chrono::time_point_cast<ToDuration>(f) == t);
    }
}

#if TEST_STD_VER > 11

template<class FromDuration, long long From, class ToDuration, long long To>
void test_constexpr ()
{
    typedef std::chrono::system_clock Clock;
    typedef std::chrono::time_point<Clock, FromDuration> FromTimePoint;
    typedef std::chrono::time_point<Clock, ToDuration> ToTimePoint;
    {
    constexpr FromTimePoint f{FromDuration{From}};
    constexpr ToTimePoint   t{ToDuration{To}};
    static_assert(std::chrono::time_point_cast<ToDuration>(f) == t, "");
    }

}

#endif

int main()
{
    test(std::chrono::milliseconds(7265000), std::chrono::hours(2));
    test(std::chrono::milliseconds(7265000), std::chrono::minutes(121));
    test(std::chrono::milliseconds(7265000), std::chrono::seconds(7265));
    test(std::chrono::milliseconds(7265000), std::chrono::milliseconds(7265000));
    test(std::chrono::milliseconds(7265000), std::chrono::microseconds(7265000000LL));
    test(std::chrono::milliseconds(7265000), std::chrono::nanoseconds(7265000000000LL));
    test(std::chrono::milliseconds(7265000),
         std::chrono::duration<double, std::ratio<3600> >(7265./3600));
    test(std::chrono::duration<int, std::ratio<2, 3> >(9),
         std::chrono::duration<int, std::ratio<3, 5> >(10));
#if TEST_STD_VER > 11
    {
    test_constexpr<std::chrono::milliseconds, 7265000, std::chrono::hours,    2> ();
    test_constexpr<std::chrono::milliseconds, 7265000, std::chrono::minutes,121> ();
    test_constexpr<std::chrono::milliseconds, 7265000, std::chrono::seconds,7265> ();
    test_constexpr<std::chrono::milliseconds, 7265000, std::chrono::milliseconds,7265000> ();
    test_constexpr<std::chrono::milliseconds, 7265000, std::chrono::microseconds,7265000000LL> ();
    test_constexpr<std::chrono::milliseconds, 7265000, std::chrono::nanoseconds,7265000000000LL> ();
    typedef std::chrono::duration<int, std::ratio<3, 5>> T1;
    test_constexpr<std::chrono::duration<int, std::ratio<2, 3>>, 9, T1, 10> ();
    }
#endif
}
