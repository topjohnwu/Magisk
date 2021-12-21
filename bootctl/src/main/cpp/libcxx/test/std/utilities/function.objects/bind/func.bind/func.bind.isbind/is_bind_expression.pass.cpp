//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <functional>

// template<class T> struct is_bind_expression

#include <functional>
#include "test_macros.h"

template <bool Expected, class T>
void
test(const T&)
{
    static_assert(std::is_bind_expression<T>::value == Expected, "");
#if TEST_STD_VER > 14
    static_assert(std::is_bind_expression_v<T> == Expected, "");
#endif
}

struct C {};

int main()
{
    test<true>(std::bind(C()));
    test<true>(std::bind(C(), std::placeholders::_2));
    test<true>(std::bind<int>(C()));
    test<false>(1);
    test<false>(std::placeholders::_2);
}
