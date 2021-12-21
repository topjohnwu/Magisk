//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// array

#include <type_traits>

template <class T>
void test_array_imp()
{
    static_assert(!std::is_reference<T>::value, "");
    static_assert(!std::is_arithmetic<T>::value, "");
    static_assert(!std::is_fundamental<T>::value, "");
    static_assert( std::is_object<T>::value, "");
    static_assert(!std::is_scalar<T>::value, "");
    static_assert( std::is_compound<T>::value, "");
    static_assert(!std::is_member_pointer<T>::value, "");
}

template <class T>
void test_array()
{
    test_array_imp<T>();
    test_array_imp<const T>();
    test_array_imp<volatile T>();
    test_array_imp<const volatile T>();
}

typedef char array[3];
typedef const char const_array[3];
typedef char incomplete_array[];

class incomplete_type;

int main()
{
    test_array<array>();
    test_array<const_array>();
    test_array<incomplete_array>();
    test_array<incomplete_type[]>();
}
