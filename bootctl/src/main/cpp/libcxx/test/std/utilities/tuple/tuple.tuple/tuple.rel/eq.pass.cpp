//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// template<class... TTypes, class... UTypes>
//   bool
//   operator==(const tuple<TTypes...>& t, const tuple<UTypes...>& u);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        typedef std::tuple<> T1;
        typedef std::tuple<> T2;
        const T1 t1;
        const T2 t2;
        assert(t1 == t2);
        assert(!(t1 != t2));
    }
    {
        typedef std::tuple<int> T1;
        typedef std::tuple<double> T2;
        const T1 t1(1);
        const T2 t2(1.1);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
    {
        typedef std::tuple<int> T1;
        typedef std::tuple<double> T2;
        const T1 t1(1);
        const T2 t2(1);
        assert(t1 == t2);
        assert(!(t1 != t2));
    }
    {
        typedef std::tuple<int, double> T1;
        typedef std::tuple<double, long> T2;
        const T1 t1(1, 2);
        const T2 t2(1, 2);
        assert(t1 == t2);
        assert(!(t1 != t2));
    }
    {
        typedef std::tuple<int, double> T1;
        typedef std::tuple<double, long> T2;
        const T1 t1(1, 2);
        const T2 t2(1, 3);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
    {
        typedef std::tuple<int, double> T1;
        typedef std::tuple<double, long> T2;
        const T1 t1(1, 2);
        const T2 t2(1.1, 2);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
    {
        typedef std::tuple<int, double> T1;
        typedef std::tuple<double, long> T2;
        const T1 t1(1, 2);
        const T2 t2(1.1, 3);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
    {
        typedef std::tuple<long, int, double> T1;
        typedef std::tuple<double, long, int> T2;
        const T1 t1(1, 2, 3);
        const T2 t2(1, 2, 3);
        assert(t1 == t2);
        assert(!(t1 != t2));
    }
    {
        typedef std::tuple<long, int, double> T1;
        typedef std::tuple<double, long, int> T2;
        const T1 t1(1, 2, 3);
        const T2 t2(1.1, 2, 3);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
    {
        typedef std::tuple<long, int, double> T1;
        typedef std::tuple<double, long, int> T2;
        const T1 t1(1, 2, 3);
        const T2 t2(1, 3, 3);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
    {
        typedef std::tuple<long, int, double> T1;
        typedef std::tuple<double, long, int> T2;
        const T1 t1(1, 2, 3);
        const T2 t2(1, 2, 4);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
    {
        typedef std::tuple<long, int, double> T1;
        typedef std::tuple<double, long, int> T2;
        const T1 t1(1, 2, 3);
        const T2 t2(1, 3, 2);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
    {
        typedef std::tuple<long, int, double> T1;
        typedef std::tuple<double, long, int> T2;
        const T1 t1(1, 2, 3);
        const T2 t2(1.1, 2, 2);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
    {
        typedef std::tuple<long, int, double> T1;
        typedef std::tuple<double, long, int> T2;
        const T1 t1(1, 2, 3);
        const T2 t2(1.1, 3, 3);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
    {
        typedef std::tuple<long, int, double> T1;
        typedef std::tuple<double, long, int> T2;
        const T1 t1(1, 2, 3);
        const T2 t2(1.1, 3, 2);
        assert(!(t1 == t2));
        assert(t1 != t2);
    }
#if TEST_STD_VER > 11
    {
        typedef std::tuple<long, int, double> T1;
        typedef std::tuple<double, long, int> T2;
        constexpr T1 t1(1, 2, 3);
        constexpr T2 t2(1.1, 3, 2);
        static_assert(!(t1 == t2), "");
        static_assert(t1 != t2, "");
    }
#endif
}
