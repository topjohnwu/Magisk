//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: c++experimental
// UNSUPPORTED: c++98, c++03

// <experimental/memory_resource>

//-----------------------------------------------------------------------------
// TESTING memory_resource * get_default_resource() noexcept;
//         memory_resource * set_default_resource(memory_resource*) noexcept;
//
// Concerns:
//  A) 'get_default_resource()' returns a non-null memory_resource pointer.
//  B) 'get_default_resource()' returns the value set by the last call to
//     'set_default_resource(...)' and 'new_delete_resource()' if no call
//     to 'set_default_resource(...)' has occurred.
//  C) 'set_default_resource(...)' returns the previous value of the default
//     resource.
//  D) 'set_default_resource(T* p)' for a non-null 'p' sets the default resource
//     to be 'p'.
//  E) 'set_default_resource(null)' sets the default resource to
//     'new_delete_resource()'.
//  F) 'get_default_resource' and 'set_default_resource' are noexcept.


#include <experimental/memory_resource>
#include <cassert>

#include "test_memory_resource.hpp"

using namespace std::experimental::pmr;

int main() {
    TestResource R;
    { // Test (A) and (B)
        memory_resource* p = get_default_resource();
        assert(p != nullptr);
        assert(p == new_delete_resource());
        assert(p == get_default_resource());
    }
    { // Test (C) and (D)
        memory_resource *expect = &R;
        memory_resource *old = set_default_resource(expect);
        assert(old != nullptr);
        assert(old == new_delete_resource());

        memory_resource *p = get_default_resource();
        assert(p != nullptr);
        assert(p == expect);
        assert(p == get_default_resource());
    }
    { // Test (E)
        memory_resource* old = set_default_resource(nullptr);
        assert(old == &R);
        memory_resource* p = get_default_resource();
        assert(p != nullptr);
        assert(p == new_delete_resource());
        assert(p == get_default_resource());
    }
    { // Test (F)
        static_assert(noexcept(get_default_resource()),
                      "get_default_resource() must be noexcept");

        static_assert(noexcept(set_default_resource(nullptr)),
                      "set_default_resource() must be noexcept");
    }
}
