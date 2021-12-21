//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template<class T> class shared_ptr
// {
// public:
//     typedef T element_type;
//     typedef weak_ptr<T> weak_type; // C++17
//     ...
// };

#include <memory>

#include "test_macros.h"

struct A;  // purposefully incomplete

int main()
{
    static_assert((std::is_same<std::shared_ptr<A>::element_type, A>::value), "");
#if TEST_STD_VER > 14
    static_assert((std::is_same<std::shared_ptr<A>::weak_type, std::weak_ptr<A>>::value), "");
#endif
}
