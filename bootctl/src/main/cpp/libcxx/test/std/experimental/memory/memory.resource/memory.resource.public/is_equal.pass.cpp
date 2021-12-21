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

//------------------------------------------------------------------------------
// TESTING virtual bool is_equal(memory_resource const &) const noexcept
//
// Concerns:
//   A) 'memory_resource' provides a function 'is_equal' with the required
//      signature.
//   B) 'is_equal' is noexcept.
//   C) 'do_is_equal' is called using the same arguments passed to 'is_equal'
//      and the resulting value is returned.
//   D) 'do_is_equal' is called on the LHS object and not the RHS object.

#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>
#include "test_memory_resource.hpp"

using std::experimental::pmr::memory_resource;

int main()
{
    {
        memory_resource const* r1 = nullptr;
        memory_resource const* r2 = nullptr;
        static_assert(
            noexcept(r1->is_equal(*r2))
          , "is_equal must be noexcept"
          );
    }
    {
        TestResource1 R1(1);
        auto& P1 = R1.getController();
        memory_resource const& M1 = R1;

        TestResource2 R2(1);
        auto& P2 = R2.getController();
        memory_resource const& M2 = R2;

        assert(M1.is_equal(M2) == false);
        assert(P1.checkIsEqualCalledEq(1));
        assert(P2.checkIsEqualCalledEq(0));

        assert(M2.is_equal(M1) == false);
        assert(P2.checkIsEqualCalledEq(1));
        assert(P1.checkIsEqualCalledEq(1));
    }
    {
        TestResource1 R1(1);
        auto& P1 = R1.getController();
        memory_resource const& M1 = R1;

        TestResource1 R2(2);
        auto& P2 = R2.getController();
        memory_resource const& M2 = R2;

        assert(M1.is_equal(M2) == false);
        assert(P1.checkIsEqualCalledEq(1));
        assert(P2.checkIsEqualCalledEq(0));

        assert(M2.is_equal(M1) == false);
        assert(P2.checkIsEqualCalledEq(1));
        assert(P1.checkIsEqualCalledEq(1));
    }
    {
        TestResource1 R1(1);
        auto& P1 = R1.getController();
        memory_resource const& M1 = R1;

        TestResource1 R2(1);
        auto& P2 = R2.getController();
        memory_resource const& M2 = R2;

        assert(M1.is_equal(M2) == true);
        assert(P1.checkIsEqualCalledEq(1));
        assert(P2.checkIsEqualCalledEq(0));

        assert(M2.is_equal(M1) == true);
        assert(P2.checkIsEqualCalledEq(1));
        assert(P1.checkIsEqualCalledEq(1));
    }
}
