//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <any>

// any() noexcept;

#include <any>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "any_helpers.h"
#include "count_new.hpp"

int main()
{
    using std::any;
    {
        static_assert(
            std::is_nothrow_default_constructible<any>::value
          , "Must be default constructible"
          );
    }
    {
        struct TestConstexpr : public std::any {
          constexpr TestConstexpr() : std::any() {}
        };
#ifdef _LIBCPP_SAFE_STATIC
        _LIBCPP_SAFE_STATIC static std::any a;
        ((void)a);
#endif
    }
    {
        DisableAllocationGuard g; ((void)g);
        any const a;
        assertEmpty(a);
    }
}
