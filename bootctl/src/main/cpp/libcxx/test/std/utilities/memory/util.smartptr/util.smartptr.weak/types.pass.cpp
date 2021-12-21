//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template<class T> class weak_ptr
// {
// public:
//     typedef T element_type;
//     ...
// };

#include <memory>

struct A;  // purposefully incomplete

int main()
{
    static_assert((std::is_same<std::weak_ptr<A>::element_type, A>::value), "");
}
