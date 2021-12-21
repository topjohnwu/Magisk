//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_base_of

#include <type_traits>

#include "test_macros.h"

template <class T, class U>
void test_is_base_of()
{
    static_assert((std::is_base_of<T, U>::value), "");
    static_assert((std::is_base_of<const T, U>::value), "");
    static_assert((std::is_base_of<T, const U>::value), "");
    static_assert((std::is_base_of<const T, const U>::value), "");
#if TEST_STD_VER > 14
    static_assert((std::is_base_of_v<T, U>), "");
    static_assert((std::is_base_of_v<const T, U>), "");
    static_assert((std::is_base_of_v<T, const U>), "");
    static_assert((std::is_base_of_v<const T, const U>), "");
#endif
}

template <class T, class U>
void test_is_not_base_of()
{
    static_assert((!std::is_base_of<T, U>::value), "");
}

struct B {};
struct B1 : B {};
struct B2 : B {};
struct D : private B1, private B2 {};

int main()
{
    test_is_base_of<B, D>();
    test_is_base_of<B1, D>();
    test_is_base_of<B2, D>();
    test_is_base_of<B, B1>();
    test_is_base_of<B, B2>();
    test_is_base_of<B, B>();

    test_is_not_base_of<D, B>();
    test_is_not_base_of<B&, D&>();
    test_is_not_base_of<B[3], D[3]>();
    test_is_not_base_of<int, int>();
}
