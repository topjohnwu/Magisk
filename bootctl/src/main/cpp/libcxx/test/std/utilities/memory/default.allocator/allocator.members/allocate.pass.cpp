//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// allocator:
// pointer allocate(size_type n, allocator<void>::const_pointer hint=0);

#include <memory>
#include <cassert>
#include <cstddef>       // for std::max_align_t
#include <iostream>

#include "test_macros.h"
#include "count_new.hpp"


#ifdef TEST_HAS_NO_ALIGNED_ALLOCATION
static const bool UsingAlignedNew = false;
#else
static const bool UsingAlignedNew = true;
#endif

#ifdef __STDCPP_DEFAULT_NEW_ALIGNMENT__
static const size_t MaxAligned = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
#else
static const size_t MaxAligned = std::alignment_of<std::max_align_t>::value;
#endif

static const size_t OverAligned = MaxAligned * 2;


template <size_t Align>
struct TEST_ALIGNAS(Align) AlignedType {
  char data;
  static int constructed;
  AlignedType() { ++constructed; }
  AlignedType(AlignedType const&) { ++constructed; }
  ~AlignedType() { --constructed; }
};
template <size_t Align>
int AlignedType<Align>::constructed = 0;


template <size_t Align>
void test_aligned() {
  typedef AlignedType<Align> T;
  T::constructed = 0;
  globalMemCounter.reset();
  std::allocator<T> a;
  const bool IsOverAlignedType = Align > MaxAligned;
  const bool ExpectAligned = IsOverAlignedType && UsingAlignedNew;
  {
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(T::constructed == 0);
    globalMemCounter.last_new_size = 0;
    globalMemCounter.last_new_align = 0;
    T* ap = a.allocate(3);
    DoNotOptimize(ap);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(globalMemCounter.checkNewCalledEq(1));
    assert(globalMemCounter.checkAlignedNewCalledEq(ExpectAligned));
    assert(globalMemCounter.checkLastNewSizeEq(3 * sizeof(T)));
    assert(globalMemCounter.checkLastNewAlignEq(ExpectAligned ? Align : 0));
    assert(T::constructed == 0);
    globalMemCounter.last_delete_align = 0;
    a.deallocate(ap, 3);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(globalMemCounter.checkDeleteCalledEq(1));
    assert(globalMemCounter.checkAlignedDeleteCalledEq(ExpectAligned));
    assert(globalMemCounter.checkLastDeleteAlignEq(ExpectAligned ? Align : 0));
    assert(T::constructed == 0);
  }
  globalMemCounter.reset();
  {
    globalMemCounter.last_new_size = 0;
    globalMemCounter.last_new_align = 0;
    T* volatile ap2 = a.allocate(11, (const void*)5);
    DoNotOptimize(ap2);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(globalMemCounter.checkNewCalledEq(1));
    assert(globalMemCounter.checkAlignedNewCalledEq(ExpectAligned));
    assert(globalMemCounter.checkLastNewSizeEq(11 * sizeof(T)));
    assert(globalMemCounter.checkLastNewAlignEq(ExpectAligned ? Align : 0));
    assert(T::constructed == 0);
    globalMemCounter.last_delete_align = 0;
    a.deallocate(ap2, 11);
    DoNotOptimize(ap2);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(globalMemCounter.checkDeleteCalledEq(1));
    assert(globalMemCounter.checkAlignedDeleteCalledEq(ExpectAligned));
    assert(globalMemCounter.checkLastDeleteAlignEq(ExpectAligned ? Align : 0));
    assert(T::constructed == 0);
  }
}

int main() {
    test_aligned<1>();
    test_aligned<2>();
    test_aligned<4>();
    test_aligned<8>();
    test_aligned<16>();
    test_aligned<MaxAligned>();
    test_aligned<OverAligned>();
    test_aligned<OverAligned * 2>();
}
