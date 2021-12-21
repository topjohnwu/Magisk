//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// void remove(const value_type& value);

#include <list>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

struct S {
  S(int i) : i_(new int(i)) {}
  S(const S &rhs) : i_(new int(*rhs.i_)) {}
  S &operator=(const S &rhs) {
    *i_ = *rhs.i_;
    return *this;
  }
  ~S() {
    delete i_;
    i_ = NULL;
  }
  bool operator==(const S &rhs) const { return *i_ == *rhs.i_; }
  int get() const { return *i_; }
  int *i_;
};

int main() {
  {
    int a1[] = {1, 2, 3, 4};
    int a2[] = {1, 2, 4};
    std::list<int> c(a1, a1 + 4);
    c.remove(3);
    assert(c == std::list<int>(a2, a2 + 3));
  }
  { // LWG issue #526
    int a1[] = {1, 2, 1, 3, 5, 8, 11};
    int a2[] = {2, 3, 5, 8, 11};
    std::list<int> c(a1, a1 + 7);
    c.remove(c.front());
    assert(c == std::list<int>(a2, a2 + 5));
  }
  {
    int a1[] = {1, 2, 1, 3, 5, 8, 11, 1};
    int a2[] = {2, 3, 5, 8, 11};
    std::list<S> c;
    for (int *ip = a1; ip < a1 + 8; ++ip)
      c.push_back(S(*ip));
    c.remove(c.front());
    std::list<S>::const_iterator it = c.begin();
    for (int *ip = a2; ip < a2 + 5; ++ip, ++it) {
      assert(it != c.end());
      assert(*ip == it->get());
    }
    assert(it == c.end());
  }
  {
    typedef no_default_allocator<int> Alloc;
    typedef std::list<int, Alloc> List;
    int a1[] = {1, 2, 3, 4};
    int a2[] = {1, 2, 4};
    List c(a1, a1 + 4, Alloc::create());
    c.remove(3);
    assert(c == List(a2, a2 + 3, Alloc::create()));
  }
#if TEST_STD_VER >= 11
  {
    int a1[] = {1, 2, 3, 4};
    int a2[] = {1, 2, 4};
    std::list<int, min_allocator<int>> c(a1, a1 + 4);
    c.remove(3);
    assert((c == std::list<int, min_allocator<int>>(a2, a2 + 3)));
  }
#endif
}
