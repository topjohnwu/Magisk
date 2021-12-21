//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// rank

#include <type_traits>

#include "test_macros.h"

template <class T, unsigned A>
void test_rank()
{
    static_assert( std::rank<T>::value == A, "");
    static_assert( std::rank<const T>::value == A, "");
    static_assert( std::rank<volatile T>::value == A, "");
    static_assert( std::rank<const volatile T>::value == A, "");
#if TEST_STD_VER > 14
    static_assert( std::rank_v<T> == A, "");
    static_assert( std::rank_v<const T> == A, "");
    static_assert( std::rank_v<volatile T> == A, "");
    static_assert( std::rank_v<const volatile T> == A, "");
#endif
}

class Class
{
public:
    ~Class();
};

int main()
{
    test_rank<void, 0>();
    test_rank<int&, 0>();
    test_rank<Class, 0>();
    test_rank<int*, 0>();
    test_rank<const int*, 0>();
    test_rank<int, 0>();
    test_rank<double, 0>();
    test_rank<bool, 0>();
    test_rank<unsigned, 0>();

    test_rank<char[3], 1>();
    test_rank<char[][3], 2>();
    test_rank<char[][4][3], 3>();
}
