//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// enum

#include <type_traits>

template <class T>
void test_enum_imp()
{
    static_assert(!std::is_reference<T>::value, "");
    static_assert(!std::is_arithmetic<T>::value, "");
    static_assert(!std::is_fundamental<T>::value, "");
    static_assert( std::is_object<T>::value, "");
    static_assert( std::is_scalar<T>::value, "");
    static_assert( std::is_compound<T>::value, "");
    static_assert(!std::is_member_pointer<T>::value, "");
}

template <class T>
void test_enum()
{
    test_enum_imp<T>();
    test_enum_imp<const T>();
    test_enum_imp<volatile T>();
    test_enum_imp<const volatile T>();
}

enum Enum {zero, one};

int main()
{
    test_enum<Enum>();
}
