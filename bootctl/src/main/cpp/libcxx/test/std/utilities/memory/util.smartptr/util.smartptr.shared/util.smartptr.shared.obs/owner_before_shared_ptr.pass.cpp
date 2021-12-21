//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// shared_ptr

// template <class U> bool owner_before(shared_ptr<U> const& b) const noexcept;

#include <memory>
#include <cassert>
#include "test_macros.h"

int main()
{
    const std::shared_ptr<int> p1(new int);
    const std::shared_ptr<int> p2 = p1;
    const std::shared_ptr<int> p3(new int);
    assert(!p1.owner_before(p2));
    assert(!p2.owner_before(p1));
    assert(p1.owner_before(p3) || p3.owner_before(p1));
    assert(p3.owner_before(p1) == p3.owner_before(p2));
    ASSERT_NOEXCEPT(p1.owner_before(p2));
}
