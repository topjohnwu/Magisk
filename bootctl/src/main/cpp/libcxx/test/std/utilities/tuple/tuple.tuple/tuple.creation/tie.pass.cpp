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

// template<class... Types>
//   tuple<Types&...> tie(Types&... t);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <string>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 11
constexpr bool test_tie_constexpr() {
    {
        int i = 42;
        double f = 1.1;
        using ExpectT = std::tuple<int&, decltype(std::ignore)&, double&>;
        auto res = std::tie(i, std::ignore, f);
        static_assert(std::is_same<ExpectT, decltype(res)>::value, "");
        assert(&std::get<0>(res) == &i);
        assert(&std::get<1>(res) == &std::ignore);
        assert(&std::get<2>(res) == &f);
        // FIXME: If/when tuple gets constexpr assignment
        //res = std::make_tuple(101, nullptr, -1.0);
    }
    return true;
}
#endif

int main()
{
    {
        int i = 0;
        std::string s;
        std::tie(i, std::ignore, s) = std::make_tuple(42, 3.14, "C++");
        assert(i == 42);
        assert(s == "C++");
    }
#if TEST_STD_VER > 11
    {
        static constexpr int i = 42;
        static constexpr double f = 1.1;
        constexpr std::tuple<const int &, const double &> t = std::tie(i, f);
        static_assert ( std::get<0>(t) == 42, "" );
        static_assert ( std::get<1>(t) == 1.1, "" );
    }
    {
        static_assert(test_tie_constexpr(), "");
    }
#endif
}
