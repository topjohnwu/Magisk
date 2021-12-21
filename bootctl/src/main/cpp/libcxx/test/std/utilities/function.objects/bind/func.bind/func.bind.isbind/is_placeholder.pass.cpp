//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// struct is_placeholder

#include <functional>
#include "test_macros.h"

template <int Expected, class T>
void
test(const T&)
{
    static_assert(std::is_placeholder<T>::value == Expected, "");
#if TEST_STD_VER > 14
    static_assert(std::is_placeholder_v<T> == Expected, "");
#endif
}

struct C {};

int main()
{
    test<1>(std::placeholders::_1);
    test<2>(std::placeholders::_2);
    test<3>(std::placeholders::_3);
    test<4>(std::placeholders::_4);
    test<5>(std::placeholders::_5);
    test<6>(std::placeholders::_6);
    test<7>(std::placeholders::_7);
    test<8>(std::placeholders::_8);
    test<9>(std::placeholders::_9);
    test<10>(std::placeholders::_10);
    test<0>(4);
    test<0>(5.5);
    test<0>('a');
    test<0>(C());
}
