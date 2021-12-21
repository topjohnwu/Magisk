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

// test swap

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

struct TT {
  int state_;
  static int count;
  TT() : state_(-1) { ++count; }
  explicit TT(int i) : state_(i) { ++count; }
  TT(const TT& a) : state_(a.state_) { ++count; }
  TT& operator=(const TT& a) {
    state_ = a.state_;
    return *this;
  }
  ~TT() { --count; }

  friend bool operator==(const TT& x, const TT& y) {
    return x.state_ == y.state_;
  }
};

int TT::count = 0;

template <class T>
typename std::remove_all_extents<T>::type* newValueInit(int size,
                                                        int new_value) {
  typedef typename std::remove_all_extents<T>::type VT;
  VT* p = newValue<T>(size);
  for (int i = 0; i < size; ++i)
    (p + i)->state_ = new_value;
  return p;
}

template <bool IsArray>
void test_basic() {
  typedef typename std::conditional<IsArray, TT[], TT>::type VT;
  const int expect_alive = IsArray ? 5 : 1;
#if TEST_STD_VER >= 11
  {
    using U = std::unique_ptr<VT, Deleter<VT> >;
    U u; ((void)u);
    ASSERT_NOEXCEPT(u.swap(u));
  }
#endif
  {
    TT* p1 = newValueInit<VT>(expect_alive, 1);
    std::unique_ptr<VT, Deleter<VT> > s1(p1, Deleter<VT>(1));
    TT* p2 = newValueInit<VT>(expect_alive, 2);
    std::unique_ptr<VT, Deleter<VT> > s2(p2, Deleter<VT>(2));
    assert(s1.get() == p1);
    assert(*s1.get() == TT(1));
    assert(s1.get_deleter().state() == 1);
    assert(s2.get() == p2);
    assert(*s2.get() == TT(2));
    assert(s2.get_deleter().state() == 2);
    s1.swap(s2);
    assert(s1.get() == p2);
    assert(*s1.get() == TT(2));
    assert(s1.get_deleter().state() == 2);
    assert(s2.get() == p1);
    assert(*s2.get() == TT(1));
    assert(s2.get_deleter().state() == 1);
    assert(TT::count == (expect_alive * 2));
  }
  assert(TT::count == 0);
}

int main() {
  test_basic</*IsArray*/ false>();
  test_basic<true>();
}
