//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <class T>
// struct pointer_traits<T*>
// {
//     static pointer pointer_to(<details>); // constexpr in C++20
//     ...
// };

#include <memory>
#include <cassert>
#include "test_macros.h"

#if TEST_STD_VER > 17
constexpr
#endif
bool check() {
    {
        int i = 0;
        static_assert((std::is_same<int *, decltype(std::pointer_traits<int*>::pointer_to(i))>::value), "");
        int* a = std::pointer_traits<int*>::pointer_to(i);
        assert(a == &i);
    }
    {
        (std::pointer_traits<void*>::element_type)0;
    }
    return true;
}

int main() {
    check();
#if TEST_STD_VER > 17
    static_assert(check(), "");
#endif
}
