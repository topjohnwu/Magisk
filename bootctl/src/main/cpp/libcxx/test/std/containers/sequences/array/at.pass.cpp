//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// reference operator[] (size_type)
// const_reference operator[] (size_type); // constexpr in C++14
// reference at (size_type)
// const_reference at (size_type); // constexpr in C++14

#include <array>
#include <cassert>

#include "test_macros.h"

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

#if TEST_STD_VER > 14
constexpr bool check_idx( size_t idx, double val )
{
    std::array<double, 3> arr = {1, 2, 3.5};
    return arr.at(idx) == val;
}
#endif

int main()
{
    {
        typedef double T;
        typedef std::array<T, 3> C;
        C c = {1, 2, 3.5};
        C::reference r1 = c.at(0);
        assert(r1 == 1);
        r1 = 5.5;
        assert(c.front() == 5.5);

        C::reference r2 = c.at(2);
        assert(r2 == 3.5);
        r2 = 7.5;
        assert(c.back() == 7.5);

#ifndef TEST_HAS_NO_EXCEPTIONS
        try
        {
            TEST_IGNORE_NODISCARD  c.at(3);
            assert(false);
        }
        catch (const std::out_of_range &) {}
#endif
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        typedef double T;
        typedef std::array<T, 0> C;
        C c = {};
        C const& cc = c;
        try
        {
            TEST_IGNORE_NODISCARD  c.at(0);
            assert(false);
        }
        catch (const std::out_of_range &) {}
        try
        {
            TEST_IGNORE_NODISCARD  cc.at(0);
            assert(false);
        }
        catch (const std::out_of_range &) {}
    }
#endif
    {
        typedef double T;
        typedef std::array<T, 3> C;
        const C c = {1, 2, 3.5};
        C::const_reference r1 = c.at(0);
        assert(r1 == 1);

        C::const_reference r2 = c.at(2);
        assert(r2 == 3.5);

#ifndef TEST_HAS_NO_EXCEPTIONS
        try
        {
            TEST_IGNORE_NODISCARD  c.at(3);
            assert(false);
        }
        catch (const std::out_of_range &) {}
#endif
    }

#if TEST_STD_VER > 11
    {
        typedef double T;
        typedef std::array<T, 3> C;
        constexpr C c = {1, 2, 3.5};

        constexpr T t1 = c.at(0);
        static_assert (t1 == 1, "");

        constexpr T t2 = c.at(2);
        static_assert (t2 == 3.5, "");
    }
#endif

#if TEST_STD_VER > 14
    {
        static_assert (check_idx(0, 1), "");
        static_assert (check_idx(1, 2), "");
        static_assert (check_idx(2, 3.5), "");
    }
#endif
}
