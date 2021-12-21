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

// template <size_t I, class... Types>
//   typename tuple_element<I, tuple<Types...> >::type&
//   get(tuple<Types...>& t);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <string>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 11

struct Empty {};

struct S {
   std::tuple<int, Empty> a;
   int k;
   Empty e;
   constexpr S() : a{1,Empty{}}, k(std::get<0>(a)), e(std::get<1>(a)) {}
   };

constexpr std::tuple<int, int> getP () { return { 3, 4 }; }
#endif

int main()
{
    {
        typedef std::tuple<int> T;
        T t(3);
        assert(std::get<0>(t) == 3);
        std::get<0>(t) = 2;
        assert(std::get<0>(t) == 2);
    }
    {
        typedef std::tuple<std::string, int> T;
        T t("high", 5);
        assert(std::get<0>(t) == "high");
        assert(std::get<1>(t) == 5);
        std::get<0>(t) = "four";
        std::get<1>(t) = 4;
        assert(std::get<0>(t) == "four");
        assert(std::get<1>(t) == 4);
    }
    {
        typedef std::tuple<double&, std::string, int> T;
        double d = 1.5;
        T t(d, "high", 5);
        assert(std::get<0>(t) == 1.5);
        assert(std::get<1>(t) == "high");
        assert(std::get<2>(t) == 5);
        std::get<0>(t) = 2.5;
        std::get<1>(t) = "four";
        std::get<2>(t) = 4;
        assert(std::get<0>(t) == 2.5);
        assert(std::get<1>(t) == "four");
        assert(std::get<2>(t) == 4);
        assert(d == 2.5);
    }
#if TEST_STD_VER > 11
    { // get on an rvalue tuple
        static_assert ( std::get<0> ( std::make_tuple ( 0.0f, 1, 2.0, 3L )) == 0, "" );
        static_assert ( std::get<1> ( std::make_tuple ( 0.0f, 1, 2.0, 3L )) == 1, "" );
        static_assert ( std::get<2> ( std::make_tuple ( 0.0f, 1, 2.0, 3L )) == 2, "" );
        static_assert ( std::get<3> ( std::make_tuple ( 0.0f, 1, 2.0, 3L )) == 3, "" );
        static_assert(S().k == 1, "");
        static_assert(std::get<1>(getP()) == 4, "");
    }
#endif

}
