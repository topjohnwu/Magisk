//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <>
// class allocator<void>
// {
// public:
//     typedef void*                                 pointer;
//     typedef const void*                           const_pointer;
//     typedef void                                  value_type;
//
//     template <class _Up> struct rebind {typedef allocator<_Up> other;};
// };

#include <memory>
#include <type_traits>

int main()
{
    static_assert((std::is_same<std::allocator<void>::pointer, void*>::value), "");
    static_assert((std::is_same<std::allocator<void>::const_pointer, const void*>::value), "");
    static_assert((std::is_same<std::allocator<void>::value_type, void>::value), "");
    static_assert((std::is_same<std::allocator<void>::rebind<int>::other,
                                std::allocator<int> >::value), "");
    std::allocator<void> a;
    std::allocator<void> a2 = a;
    a2 = a;
}
