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
// TESTING virtual ~memory_resource()
//
// Concerns:
//  A) 'memory_resource' is destructible.
//  B) The destructor is implicitly marked noexcept.
//  C) The destructor is marked virtual.

#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

#include "test_memory_resource.hpp"

using std::experimental::pmr::memory_resource;

int main()
{
    static_assert(
        std::has_virtual_destructor<memory_resource>::value
      , "Must have virtual destructor"
      );
    static_assert(
        std::is_nothrow_destructible<memory_resource>::value,
        "Must be nothrow destructible"
      );
    static_assert(
        std::is_abstract<memory_resource>::value
      , "Must be abstract"
      );
    // Check that the destructor of `TestResource` is called when
    // it is deleted as a pointer to `memory_resource`
    {
        using TR = TestResource;
        memory_resource* M = new TR(42);
        assert(TR::resource_alive == 1);
        assert(TR::resource_constructed == 1);
        assert(TR::resource_destructed == 0);

        delete M;

        assert(TR::resource_alive == 0);
        assert(TR::resource_constructed == 1);
        assert(TR::resource_destructed == 1);
    }
}
