//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <memory>

// unique_ptr

// FIXME(EricWF): This test contains tests for constructing a unique_ptr from NULL.
// The behavior demonstrated in this test is not meant to be standard; It simply
// tests the current status quo in libc++.

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

template <class VT>
void test_pointer_ctor() {
  {
    std::unique_ptr<VT> p(0);
    assert(p.get() == 0);
  }
  {
    std::unique_ptr<VT, Deleter<VT> > p(0);
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 0);
  }
}

template <class VT>
void test_pointer_deleter_ctor() {
  {
    std::default_delete<VT> d;
    std::unique_ptr<VT> p(0, d);
    assert(p.get() == 0);
  }
  {
    std::unique_ptr<VT, Deleter<VT> > p(0, Deleter<VT>(5));
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 5);
  }
  {
    NCDeleter<VT> d(5);
    std::unique_ptr<VT, NCDeleter<VT>&> p(0, d);
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 5);
  }
  {
    NCConstDeleter<VT> d(5);
    std::unique_ptr<VT, NCConstDeleter<VT> const&> p(0, d);
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 5);
  }
}

int main() {
  {
    // test_pointer_ctor<int>();
    test_pointer_deleter_ctor<int>();
  }
  {
    test_pointer_ctor<int[]>();
    test_pointer_deleter_ctor<int[]>();
  }
}
