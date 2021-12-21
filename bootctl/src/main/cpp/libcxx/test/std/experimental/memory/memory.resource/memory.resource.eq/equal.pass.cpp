//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <experimental/memory_resource>

// bool operator==(memory_resource const &, memory_resource const &) noexcept;

#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_memory_resource.hpp"

namespace ex = std::experimental::pmr;

int main()
{
    // check return types
    {
        ex::memory_resource const * mr1(nullptr);
        ex::memory_resource const * mr2(nullptr);
        static_assert(std::is_same<decltype(*mr1 == *mr2), bool>::value, "");
        static_assert(noexcept(*mr1 == *mr2), "");
    }
    // equal
    {
        TestResource r1(1);
        TestResource r2(1);
        ex::memory_resource const & mr1 = r1;
        ex::memory_resource const & mr2 = r2;

        assert(mr1 == mr2);
        assert(r1.checkIsEqualCalledEq(1));
        assert(r2.checkIsEqualCalledEq(0));

        assert(mr2 == mr1);
        assert(r1.checkIsEqualCalledEq(1));
        assert(r2.checkIsEqualCalledEq(1));
    }
    // equal same object
    {
        TestResource r1(1);
        ex::memory_resource const & mr1 = r1;
        ex::memory_resource const & mr2 = r1;

        assert(mr1 == mr2);
        assert(r1.checkIsEqualCalledEq(0));

        assert(mr2 == mr1);
        assert(r1.checkIsEqualCalledEq(0));
    }
    // not equal
    {
        TestResource r1(1);
        TestResource r2(2);
        ex::memory_resource const & mr1 = r1;
        ex::memory_resource const & mr2 = r2;

        assert(!(mr1 == mr2));
        assert(r1.checkIsEqualCalledEq(1));
        assert(r2.checkIsEqualCalledEq(0));

        assert(!(mr2 == mr1));
        assert(r1.checkIsEqualCalledEq(1));
        assert(r2.checkIsEqualCalledEq(1));
    }
}
