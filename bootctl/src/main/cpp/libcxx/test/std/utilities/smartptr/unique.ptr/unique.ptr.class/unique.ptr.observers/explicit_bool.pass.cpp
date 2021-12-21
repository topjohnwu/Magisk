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

// test op*()

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

template <class UPtr>
void doTest(UPtr& p, bool ExpectTrue) {
  if (p)
    assert(ExpectTrue);
  else
    assert(!ExpectTrue);

  if (!p)
    assert(!ExpectTrue);
  else
    assert(ExpectTrue);
}

template <bool IsArray>
void test_basic() {
  typedef typename std::conditional<IsArray, int[], int>::type VT;
  typedef std::unique_ptr<VT> U;
  {
    static_assert((std::is_constructible<bool, U>::value), "");
    static_assert((std::is_constructible<bool, U const&>::value), "");
  }
#if TEST_STD_VER >= 11
  {
    static_assert(!std::is_convertible<U, bool>::value, "");
    static_assert(!std::is_convertible<U const&, bool>::value, "");
  }
#endif
  {
    U p(newValue<VT>(1));
    U const& cp = p;
    doTest(p, true);
    doTest(cp, true);
  }
  {
    U p;
    const U& cp = p;
    doTest(p, false);
    doTest(cp, false);
  }
}

int main() {
  test_basic</*IsArray*/ false>();
  test_basic<true>();
}
