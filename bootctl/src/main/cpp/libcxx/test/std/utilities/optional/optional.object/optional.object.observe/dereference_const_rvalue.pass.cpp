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

// constexpr T&& optional<T>::operator*() const &&;

#ifdef _LIBCPP_DEBUG
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

using std::optional;

struct X
{
    constexpr int test() const& {return 3;}
    int test() & {return 4;}
    constexpr int test() const&& {return 5;}
    int test() && {return 6;}
};

struct Y
{
    int test() const && {return 2;}
};

int main()
{
    {
        const optional<X> opt; ((void)opt);
        ASSERT_SAME_TYPE(decltype(*std::move(opt)), X const &&);
        // ASSERT_NOT_NOEXCEPT(*std::move(opt));
        // FIXME: This assertion fails with GCC because it can see that
        // (A) operator*() is constexpr, and
        // (B) there is no path through the function that throws.
        // It's arguable if this is the correct behavior for the noexcept
        // operator.
        // Regardless this function should still be noexcept(false) because
        // it has a narrow contract.
    }
    {
        constexpr optional<X> opt(X{});
        static_assert((*std::move(opt)).test() == 5, "");
    }
    {
        constexpr optional<Y> opt(Y{});
        assert((*std::move(opt)).test() == 2);
    }
#ifdef _LIBCPP_DEBUG
    {
        optional<X> opt;
        assert((*std::move(opt)).test() == 5);
        assert(false);
    }
#endif  // _LIBCPP_DEBUG
}
