//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <optional>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: clang-5, apple-clang-9
// UNSUPPORTED: libcpp-no-deduction-guides
// Clang 5 will generate bad implicit deduction guides
//  Specifically, for the copy constructor.


// template<class T>
//   optional(T) -> optional<T>;


#include <optional>
#include <cassert>

struct A {};

int main()
{
//  Test the explicit deduction guides
    {
//  optional(T)
    std::optional opt(5);
    static_assert(std::is_same_v<decltype(opt), std::optional<int>>, "");
    assert(static_cast<bool>(opt));
    assert(*opt == 5);
    }

    {
//  optional(T)
    std::optional opt(A{});
    static_assert(std::is_same_v<decltype(opt), std::optional<A>>, "");
    assert(static_cast<bool>(opt));
    }

//  Test the implicit deduction guides
    {
//  optional(optional);
    std::optional<char> source('A');
    std::optional opt(source);
    static_assert(std::is_same_v<decltype(opt), std::optional<char>>, "");
    assert(static_cast<bool>(opt) == static_cast<bool>(source));
    assert(*opt == *source);
    }
}
