//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// duration

// duration() = default;

// Rep must be default initialized, not initialized with 0

#include <chrono>
#include <cassert>

#include "test_macros.h"
#include "../../rep.h"

template <class D>
void
test()
{
    D d;
    assert(d.count() == typename D::rep());
#if TEST_STD_VER >= 11
    constexpr D d2 = D();
    static_assert(d2.count() == typename D::rep(), "");
#endif
}

int main()
{
    test<std::chrono::duration<Rep> >();
}
