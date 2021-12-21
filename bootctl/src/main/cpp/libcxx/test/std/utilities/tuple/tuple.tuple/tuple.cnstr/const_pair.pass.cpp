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

// template <class U1, class U2> tuple(const pair<U1, U2>& u);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <utility>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        typedef std::pair<long, char> T0;
        typedef std::tuple<long long, short> T1;
        T0 t0(2, 'a');
        T1 t1 = t0;
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1) == short('a'));
    }
#if TEST_STD_VER > 11
    {
        typedef std::pair<long, char> P0;
        typedef std::tuple<long long, short> T1;
        constexpr P0 p0(2, 'a');
        constexpr T1 t1 = p0;
        static_assert(std::get<0>(t1) == std::get<0>(p0), "");
        static_assert(std::get<1>(t1) == std::get<1>(p0), "");
        static_assert(std::get<0>(t1) == 2, "");
        static_assert(std::get<1>(t1) == short('a'), "");
    }
#endif
}
