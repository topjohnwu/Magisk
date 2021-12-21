//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// <optional>

// struct nullopt_t{see below};
// inline constexpr nullopt_t nullopt(unspecified);

// [optional.nullopt]/2:
//   Type nullopt_t shall not have a default constructor or an initializer-list
//   constructor, and shall not be an aggregate.

#include <optional>
#include <type_traits>

using std::nullopt_t;
using std::nullopt;

constexpr bool test()
{
    nullopt_t foo{nullopt};
    (void)foo;
    return true;
}

int main()
{
    static_assert(std::is_empty_v<nullopt_t>);
    static_assert(!std::is_default_constructible_v<nullopt_t>);

    static_assert(std::is_same_v<const nullopt_t, decltype(nullopt)>);
    static_assert(test());
}
