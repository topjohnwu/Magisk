//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template <class T1, class T2> struct pair

// template<size_t I, class T1, class T2>
//     const typename tuple_element<I, std::pair<T1, T2> >::type&
//     get(const pair<T1, T2>&);

#include <utility>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        typedef std::pair<int, short> P;
        const P p(3, static_cast<short>(4));
        assert(std::get<0>(p) == 3);
        assert(std::get<1>(p) == 4);
    }

#if TEST_STD_VER > 11
    {
        typedef std::pair<int, short> P;
        constexpr P p1(3, static_cast<short>(4));
        static_assert(std::get<0>(p1) == 3, "");
        static_assert(std::get<1>(p1) == 4, "");
    }
#endif
}
