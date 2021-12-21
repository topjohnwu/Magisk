//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template <class T> typename add_rvalue_reference<T>::type declval() noexcept;

#include <utility>
#include <type_traits>

#include "test_macros.h"

class A
{
    A(const A&);
    A& operator=(const A&);
};

int main()
{
#if TEST_STD_VER >= 11
    static_assert((std::is_same<decltype(std::declval<A>()), A&&>::value), "");
#else
    static_assert((std::is_same<decltype(std::declval<A>()), A&>::value), "");
#endif
}
