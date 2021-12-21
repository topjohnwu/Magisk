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
//     typedef T element_type;
//     ...
// };

#include <memory>
#include <type_traits>

int main()
{
    static_assert((std::is_same<std::pointer_traits<const short*>::element_type, const short>::value), "");
}
