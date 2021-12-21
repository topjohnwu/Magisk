//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// reference_wrapper

// check for member typedef type

#include <functional>
#include <type_traits>

class C {};

int main()
{
    static_assert((std::is_same<std::reference_wrapper<C>::type,
                                                       C>::value), "");
    static_assert((std::is_same<std::reference_wrapper<void ()>::type,
                                                       void ()>::value), "");
    static_assert((std::is_same<std::reference_wrapper<int* (double*)>::type,
                                                       int* (double*)>::value), "");
    static_assert((std::is_same<std::reference_wrapper<void(*)()>::type,
                                                       void(*)()>::value), "");
    static_assert((std::is_same<std::reference_wrapper<int*(*)(double*)>::type,
                                                       int*(*)(double*)>::value), "");
    static_assert((std::is_same<std::reference_wrapper<int*(C::*)(double*)>::type,
                                                       int*(C::*)(double*)>::value), "");
    static_assert((std::is_same<std::reference_wrapper<int (C::*)(double*) const volatile>::type,
                                                       int (C::*)(double*) const volatile>::value), "");
}
