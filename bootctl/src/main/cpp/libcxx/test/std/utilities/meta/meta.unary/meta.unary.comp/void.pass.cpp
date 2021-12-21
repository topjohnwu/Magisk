//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// void

#include <type_traits>

template <class T>
void test_void_imp()
{
    static_assert(!std::is_reference<T>::value, "");
    static_assert(!std::is_arithmetic<T>::value, "");
    static_assert( std::is_fundamental<T>::value, "");
    static_assert(!std::is_object<T>::value, "");
    static_assert(!std::is_scalar<T>::value, "");
    static_assert(!std::is_compound<T>::value, "");
    static_assert(!std::is_member_pointer<T>::value, "");
}

template <class T>
void test_void()
{
    test_void_imp<T>();
    test_void_imp<const T>();
    test_void_imp<volatile T>();
    test_void_imp<const volatile T>();
}

int main()
{
    test_void<void>();
}
