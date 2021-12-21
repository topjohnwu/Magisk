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

// optional(const optional<T>& rhs);

#include <optional>
#include <string>
#include <type_traits>

using std::optional;

struct X {};

struct Y
{
    Y() = default;
    Y(const Y&) {}
};

struct Z
{
    Z() = default;
    Z(Z&&) = delete;
    Z(const Z&) = delete;
    Z& operator=(Z&&) = delete;
    Z& operator=(const Z&) = delete;
};

int main()
{
    {
        using T = int;
        static_assert((std::is_trivially_copy_constructible<optional<T>>::value), "");
        constexpr optional<T> opt;
        constexpr optional<T> opt2 = opt;
        (void)opt2;
    }
    {
        using T = X;
        static_assert((std::is_trivially_copy_constructible<optional<T>>::value), "");
        constexpr optional<T> opt;
        constexpr optional<T> opt2 = opt;
        (void)opt2;
    }
    static_assert(!(std::is_trivially_copy_constructible<optional<Y>>::value), "");
    static_assert(!(std::is_trivially_copy_constructible<optional<std::string>>::value), "");

    static_assert(!(std::is_copy_constructible<optional<Z>>::value), "");
}
