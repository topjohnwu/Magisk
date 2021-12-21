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

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// constexpr T& optional<T>::value() &&;

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

using std::optional;
using std::bad_optional_access;

struct X
{
    X() = default;
    X(const X&) = delete;
    constexpr int test() const & {return 3;}
    int test() & {return 4;}
    constexpr int test() const && {return 5;}
    int test() && {return 6;}
};

struct Y
{
    constexpr int test() && {return 7;}
};

constexpr int
test()
{
    optional<Y> opt{Y{}};
    return std::move(opt).value().test();
}

int main()
{
    {
        optional<X> opt; ((void)opt);
        ASSERT_NOT_NOEXCEPT(std::move(opt).value());
        ASSERT_SAME_TYPE(decltype(std::move(opt).value()), X&&);
    }
    {
        optional<X> opt;
        opt.emplace();
        assert(std::move(opt).value().test() == 6);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        optional<X> opt;
        try
        {
            (void)std::move(opt).value();
            assert(false);
        }
        catch (const bad_optional_access&)
        {
        }
    }
#endif
    static_assert(test() == 7, "");
}
