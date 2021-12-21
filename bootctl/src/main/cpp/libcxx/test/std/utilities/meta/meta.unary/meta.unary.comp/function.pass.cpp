//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// function

#include <type_traits>

template <class T>
void test_function_imp()
{
    static_assert(!std::is_reference<T>::value, "");
    static_assert(!std::is_arithmetic<T>::value, "");
    static_assert(!std::is_fundamental<T>::value, "");
    static_assert(!std::is_object<T>::value, "");
    static_assert(!std::is_scalar<T>::value, "");
    static_assert( std::is_compound<T>::value, "");
    static_assert(!std::is_member_pointer<T>::value, "");
}

template <class T>
void test_function()
{
    test_function_imp<T>();
    test_function_imp<const T>();
    test_function_imp<volatile T>();
    test_function_imp<const volatile T>();
}

int main()
{
    test_function<void ()>();
    test_function<void (int)>();
    test_function<int (double)>();
    test_function<int (double, char)>();
}
