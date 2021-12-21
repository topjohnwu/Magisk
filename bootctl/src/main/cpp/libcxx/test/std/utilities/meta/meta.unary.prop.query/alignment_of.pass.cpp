//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// alignment_of

#include <type_traits>
#include <cstdint>

#include "test_macros.h"

template <class T, unsigned A>
void test_alignment_of()
{
    static_assert( std::alignment_of<T>::value == A, "");
    static_assert( std::alignment_of<const T>::value == A, "");
    static_assert( std::alignment_of<volatile T>::value == A, "");
    static_assert( std::alignment_of<const volatile T>::value == A, "");
#if TEST_STD_VER > 14
    static_assert( std::alignment_of_v<T> == A, "");
    static_assert( std::alignment_of_v<const T> == A, "");
    static_assert( std::alignment_of_v<volatile T> == A, "");
    static_assert( std::alignment_of_v<const volatile T> == A, "");
#endif
}

class Class
{
public:
    ~Class();
};

int main()
{
    test_alignment_of<int&, 4>();
    test_alignment_of<Class, 1>();
    test_alignment_of<int*, sizeof(intptr_t)>();
    test_alignment_of<const int*, sizeof(intptr_t)>();
    test_alignment_of<char[3], 1>();
    test_alignment_of<int, 4>();
    test_alignment_of<double, 8>();
#if (defined(__ppc__) && !defined(__ppc64__))
    test_alignment_of<bool, 4>();   // 32-bit PPC has four byte bool
#else
    test_alignment_of<bool, 1>();
#endif
    test_alignment_of<unsigned, 4>();
}
