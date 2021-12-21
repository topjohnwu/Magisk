//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <experimental/memory_resource>

// UNSUPPORTED: c++98, c++03

//------------------------------------------------------------------------------
// TESTING void * memory_resource::allocate(size_t, size_t = max_align)
//
// Concerns:
//  A) 'memory_resource' contains a member 'allocate' with the required
//     signature, including the default alignment parameter.
//  B) The return type of 'allocate' is 'void*'.
//  C) 'allocate' is not marked as 'noexcept'.
//  D) Invoking 'allocate' invokes 'do_allocate' with the same arguments.
//  E) If 'do_allocate' throws then 'allocate' propagates that exception.

#include <experimental/memory_resource>
#include <type_traits>
#include <cstddef>
#include <cassert>

#include "test_macros.h"
#include "test_memory_resource.hpp"

using std::experimental::pmr::memory_resource;

int main()
{
    TestResource R(42);
    auto& P = R.getController();
    memory_resource& M = R;
    {
        static_assert(
            std::is_same<decltype(M.allocate(0, 0)), void*>::value
          , "Must be void*"
          );
        static_assert(
            std::is_same<decltype(M.allocate(0)), void*>::value
          , "Must be void*"
          );
    }
    {
        static_assert(
            ! noexcept(M.allocate(0, 0))
          , "Must not be noexcept."
          );
        static_assert(
            ! noexcept(M.allocate(0))
          , "Must not be noexcept."
          );
    }
    {
        int s = 42;
        int a = 64;
        void* p = M.allocate(s, a);
        assert(P.alloc_count == 1);
        assert(P.checkAlloc(p, s, a));

        s = 128;
        a = MaxAlignV;
        p = M.allocate(s);
        assert(P.alloc_count == 2);
        assert(P.checkAlloc(p, s, a));
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        TestResource R2;
        auto& P2 = R2.getController();
        P2.throw_on_alloc = true;
        memory_resource& M2 = R2;
        try {
            M2.allocate(42);
            assert(false);
        } catch (TestException const&) {
            // do nothing.
        } catch (...) {
            assert(false);
        }
    }
#endif
}
