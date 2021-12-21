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

// tuple(const tuple& u) = default;

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <string>
#include <cassert>

#include "test_macros.h"

struct Empty {};

int main()
{
    {
        typedef std::tuple<> T;
        T t0;
        T t = t0;
        ((void)t); // Prevent unused warning
    }
    {
        typedef std::tuple<int> T;
        T t0(2);
        T t = t0;
        assert(std::get<0>(t) == 2);
    }
    {
        typedef std::tuple<int, char> T;
        T t0(2, 'a');
        T t = t0;
        assert(std::get<0>(t) == 2);
        assert(std::get<1>(t) == 'a');
    }
    {
        typedef std::tuple<int, char, std::string> T;
        const T t0(2, 'a', "some text");
        T t = t0;
        assert(std::get<0>(t) == 2);
        assert(std::get<1>(t) == 'a');
        assert(std::get<2>(t) == "some text");
    }
#if TEST_STD_VER > 11
    {
        typedef std::tuple<int> T;
        constexpr T t0(2);
        constexpr T t = t0;
        static_assert(std::get<0>(t) == 2, "");
    }
    {
        typedef std::tuple<Empty> T;
        constexpr T t0;
        constexpr T t = t0;
        constexpr Empty e = std::get<0>(t);
        ((void)e); // Prevent unused warning
    }
#endif
}
