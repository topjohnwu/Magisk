//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// decay

#include <type_traits>

#include "test_macros.h"

template <class T, class U>
void test_decay()
{
    static_assert((std::is_same<typename std::decay<T>::type, U>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::decay_t<T>,     U>::value), "");
#endif
}

int main()
{
    test_decay<void, void>();
    test_decay<int, int>();
    test_decay<const volatile int, int>();
    test_decay<int*, int*>();
    test_decay<int[3], int*>();
    test_decay<const int[3], const int*>();
    test_decay<void(), void (*)()>();
#if TEST_STD_VER > 11
    test_decay<int(int) const, int(int) const>();
    test_decay<int(int) volatile, int(int) volatile>();
    test_decay<int(int)  &, int(int)  &>();
    test_decay<int(int) &&, int(int) &&>();
#endif
}
