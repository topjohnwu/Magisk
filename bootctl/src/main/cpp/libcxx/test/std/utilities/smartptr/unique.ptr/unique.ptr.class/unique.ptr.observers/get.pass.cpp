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

// test get

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

template <bool IsArray>
void test_basic() {
  typedef typename std::conditional<IsArray, int[], int>::type VT;
  typedef const VT CVT;
  {
    typedef std::unique_ptr<VT> U;
    int* p = newValue<VT>(1);
    U s(p);
    U const& sc = s;
    ASSERT_SAME_TYPE(decltype(s.get()), int*);
    ASSERT_SAME_TYPE(decltype(sc.get()), int*);
    assert(s.get() == p);
    assert(sc.get() == s.get());
  }
  {
    typedef std::unique_ptr<CVT> U;
    const int* p = newValue<VT>(1);
    U s(p);
    U const& sc = s;
    ASSERT_SAME_TYPE(decltype(s.get()), const int*);
    ASSERT_SAME_TYPE(decltype(sc.get()), const int*);
    assert(s.get() == p);
    assert(sc.get() == s.get());
  }
}

int main() {
  test_basic</*IsArray*/ false>();
  test_basic<true>();
}
