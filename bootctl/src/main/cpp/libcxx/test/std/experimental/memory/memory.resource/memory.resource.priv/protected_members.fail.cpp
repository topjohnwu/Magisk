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

// memory_resource::do_allocate(size_t, size_t);          /* protected */
// memory_resource::do_deallocate(void*, size_t, size_t); /* protected */
// memory_resource::do_is_equal(memory_resource const&);  /* protected */

#include <experimental/memory_resource>

namespace ex = std::experimental::pmr;

int main() {
    ex::memory_resource *m = ex::new_delete_resource();
    m->do_allocate(0, 0); // expected-error{{'do_allocate' is a protected member}}
    m->do_deallocate(nullptr, 0, 0); // expected-error{{'do_deallocate' is a protected member}}
    m->do_is_equal(*m); // expected-error{{'do_is_equal' is a protected member}}
}
