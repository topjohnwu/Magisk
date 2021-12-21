//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// template <class Clock, class Duration1, class Duration2>
// struct common_type<chrono::time_point<Clock, Duration1>, chrono::time_point<Clock, Duration2>>
// {
//     typedef chrono::time_point<Clock, typename common_type<Duration1, Duration2>::type> type;
// };

#include <chrono>

template <class D1, class D2, class De>
void
test()
{
    typedef std::chrono::system_clock C;
    typedef std::chrono::time_point<C, D1> T1;
    typedef std::chrono::time_point<C, D2> T2;
    typedef std::chrono::time_point<C, De> Te;
    typedef typename std::common_type<T1, T2>::type Tc;
    static_assert((std::is_same<Tc, Te>::value), "");
}

int main()
{
    test<std::chrono::duration<int, std::ratio<1, 100> >,
         std::chrono::duration<long, std::ratio<1, 1000> >,
         std::chrono::duration<long, std::ratio<1, 1000> > >();
    test<std::chrono::duration<long, std::ratio<1, 100> >,
         std::chrono::duration<int, std::ratio<1, 1000> >,
         std::chrono::duration<long, std::ratio<1, 1000> > >();
    test<std::chrono::duration<char, std::ratio<1, 30> >,
         std::chrono::duration<short, std::ratio<1, 1000> >,
         std::chrono::duration<int, std::ratio<1, 3000> > >();
    test<std::chrono::duration<double, std::ratio<21, 1> >,
         std::chrono::duration<short, std::ratio<15, 1> >,
         std::chrono::duration<double, std::ratio<3, 1> > >();
}
