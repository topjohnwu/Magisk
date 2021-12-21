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

// constexpr optional() noexcept;

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "archetypes.hpp"

using std::optional;

template <class Opt>
void
test_constexpr()
{
    static_assert(std::is_nothrow_default_constructible<Opt>::value, "");
    static_assert(std::is_trivially_destructible<Opt>::value, "");
    static_assert(std::is_trivially_destructible<typename Opt::value_type>::value, "");

    constexpr Opt opt;
    static_assert(static_cast<bool>(opt) == false, "");

    struct test_constexpr_ctor
        : public Opt
    {
        constexpr test_constexpr_ctor() {}
    };
}

template <class Opt>
void
test()
{
    static_assert(std::is_nothrow_default_constructible<Opt>::value, "");
    static_assert(!std::is_trivially_destructible<Opt>::value, "");
    static_assert(!std::is_trivially_destructible<typename Opt::value_type>::value, "");
    {
        Opt opt;
        assert(static_cast<bool>(opt) == false);
    }
    {
        const Opt opt;
        assert(static_cast<bool>(opt) == false);
    }

    struct test_constexpr_ctor
        : public Opt
    {
        constexpr test_constexpr_ctor() {}
    };
}

int main()
{
    test_constexpr<optional<int>>();
    test_constexpr<optional<int*>>();
    test_constexpr<optional<ImplicitTypes::NoCtors>>();
    test_constexpr<optional<NonTrivialTypes::NoCtors>>();
    test_constexpr<optional<NonConstexprTypes::NoCtors>>();
    test<optional<NonLiteralTypes::NoCtors>>();
    // EXTENSIONS
#if defined(_LIBCPP_VERSION) && 0 // FIXME these extensions are currently disabled.
    test_constexpr<optional<int&>>();
    test_constexpr<optional<const int&>>();
    test_constexpr<optional<int&>>();
    test_constexpr<optional<NonLiteralTypes::NoCtors&>>();
    test_constexpr<optional<NonLiteralTypes::NoCtors&&>>();
#endif
}
