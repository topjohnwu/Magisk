//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// unique_ptr

// test release

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

template <bool IsArray>
void test_basic() {
  typedef typename std::conditional<IsArray, A[], A>::type VT;
  const int expect_alive = IsArray ? 3 : 1;
#if TEST_STD_VER >= 11
  {
    using U = std::unique_ptr<VT>;
    U u; ((void)u);
    ASSERT_NOEXCEPT(u.release());
  }
#endif
  {
    std::unique_ptr<VT> p(newValue<VT>(expect_alive));
    assert(A::count == expect_alive);
    A* ap = p.get();
    A* a = p.release();
    assert(A::count == expect_alive);
    assert(p.get() == nullptr);
    assert(ap == a);
    assert(a != nullptr);

    if (IsArray)
      delete[] a;
    else
      delete a;

    assert(A::count == 0);
  }
  assert(A::count == 0);
}

int main() {
  test_basic</*IsArray*/ false>();
  test_basic<true>();
}
