//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// [func.require]

#include <type_traits>
#include <functional>

#include "test_macros.h"

template <typename T, int N>
struct Array
{
    typedef T type[N];
};

struct Type
{
    Array<char, 1>::type& f1();
    Array<char, 2>::type& f2() const;
#if TEST_STD_VER >= 11
    Array<char, 1>::type& g1()        &;
    Array<char, 2>::type& g2() const  &;
    Array<char, 3>::type& g3()       &&;
    Array<char, 4>::type& g4() const &&;
#endif
};

int main()
{
    static_assert(sizeof(std::__invoke(&Type::f1, std::declval<Type        >())) == 1, "");
    static_assert(sizeof(std::__invoke(&Type::f2, std::declval<Type const  >())) == 2, "");
#if TEST_STD_VER >= 11
    static_assert(sizeof(std::__invoke(&Type::g1, std::declval<Type       &>())) == 1, "");
    static_assert(sizeof(std::__invoke(&Type::g2, std::declval<Type const &>())) == 2, "");
    static_assert(sizeof(std::__invoke(&Type::g3, std::declval<Type      &&>())) == 3, "");
    static_assert(sizeof(std::__invoke(&Type::g4, std::declval<Type const&&>())) == 4, "");
#endif
}
