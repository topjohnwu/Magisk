//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>

// explicit forward_list(const allocator_type& a);

#include <forward_list>
#include <cassert>

#include "test_allocator.h"
#include "../../../NotConstructible.h"

int main()
{
    {
        typedef test_allocator<NotConstructible> A;
        typedef A::value_type T;
        typedef std::forward_list<T, A> C;
        C c = A(12);
        assert(c.get_allocator() == A(12));
        assert(c.empty());
    }
}
