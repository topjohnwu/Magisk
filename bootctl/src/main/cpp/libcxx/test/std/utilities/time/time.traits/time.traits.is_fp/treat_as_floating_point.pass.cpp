//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// treat_as_floating_point

#include <chrono>
#include <type_traits>

#include "test_macros.h"

template <class T>
void
test()
{
    static_assert((std::is_base_of<std::is_floating_point<T>,
                                   std::chrono::treat_as_floating_point<T> >::value), "");
#if TEST_STD_VER > 14
    static_assert(std::is_floating_point<T>::value ==
                                  std::chrono::treat_as_floating_point_v<T>, "");
#endif
}

struct A {};

int main()
{
    test<int>();
    test<unsigned>();
    test<char>();
    test<bool>();
    test<float>();
    test<double>();
    test<long double>();
    test<A>();
}
