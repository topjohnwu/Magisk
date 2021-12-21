//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// template <class InputIter> vector(InputIter first, InputIter last,
//                                   const allocator_type& a);

#include <vector>
#include <cassert>

#include "min_allocator.h"

void test_ctor_under_alloc() {
  int arr1[] = {42};
  int arr2[] = {1, 101, 42};
  {
    typedef std::vector<int, cpp03_allocator<int> > C;
    typedef C::allocator_type Alloc;
    Alloc a;
    {
      Alloc::construct_called = false;
      C v(arr1, arr1 + 1, a);
      assert(Alloc::construct_called);
    }
    {
      Alloc::construct_called = false;
      C v(arr2, arr2 + 3, a);
      assert(Alloc::construct_called);
    }
  }
  {
    typedef std::vector<int, cpp03_overload_allocator<int> > C;
    typedef C::allocator_type Alloc;
    Alloc a;
    {
      Alloc::construct_called = false;
      C v(arr1, arr1 + 1, a);
      assert(Alloc::construct_called);
    }
    {
      Alloc::construct_called = false;
      C v(arr2, arr2 + 3, a);
      assert(Alloc::construct_called);
    }
  }
}

int main() {
  test_ctor_under_alloc();
}
