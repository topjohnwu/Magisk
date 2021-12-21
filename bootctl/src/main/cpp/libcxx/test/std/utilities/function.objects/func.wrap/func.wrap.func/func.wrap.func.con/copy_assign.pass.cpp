//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// class function<R(ArgTypes...)>

// function& operator=(const function& f);

#include <functional>
#include <cassert>

#include "test_macros.h"
#include "count_new.hpp"

class A {
  int data_[10];

public:
  static int count;

  A() {
    ++count;
    for (int i = 0; i < 10; ++i)
      data_[i] = i;
  }

  A(const A &) { ++count; }

  ~A() { --count; }

  int operator()(int i) const {
    for (int j = 0; j < 10; ++j)
      i += data_[j];
    return i;
  }
};

int A::count = 0;

int g0() { return 0; }
int g(int) { return 0; }
int g2(int, int) { return 2; }
int g3(int, int, int) { return 3; }

int main() {
  assert(globalMemCounter.checkOutstandingNewEq(0));
  {
    std::function<int(int)> f = A();
    assert(A::count == 1);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(f.target<A>());
    assert(f.target<int (*)(int)>() == 0);
    std::function<int(int)> f2;
    f2 = f;
    assert(A::count == 2);
    assert(globalMemCounter.checkOutstandingNewEq(2));
    assert(f2.target<A>());
    assert(f2.target<int (*)(int)>() == 0);
  }
  assert(A::count == 0);
  assert(globalMemCounter.checkOutstandingNewEq(0));
  {
    std::function<int(int)> f = g;
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(f.target<int (*)(int)>());
    assert(f.target<A>() == 0);
    std::function<int(int)> f2;
    f2 = f;
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(f2.target<int (*)(int)>());
    assert(f2.target<A>() == 0);
  }
  assert(globalMemCounter.checkOutstandingNewEq(0));
  {
    std::function<int(int)> f;
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(f.target<int (*)(int)>() == 0);
    assert(f.target<A>() == 0);
    std::function<int(int)> f2;
    f2 = f;
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(f2.target<int (*)(int)>() == 0);
    assert(f2.target<A>() == 0);
  }
  {
    typedef std::function<int()> Func;
    Func f = g0;
    Func& fr = (f = (Func &)f);
    assert(&fr == &f);
    assert(*f.target<int(*)()>() == g0);
  }
  {
    typedef std::function<int(int)> Func;
    Func f = g;
    Func& fr = (f = (Func &)f);
    assert(&fr == &f);
    assert(*f.target<int(*)(int)>() == g);
  }
  {
    typedef std::function<int(int, int)> Func;
    Func f = g2;
    Func& fr = (f = (Func &)f);
    assert(&fr == &f);
    assert(*f.target<int(*)(int, int)>() == g2);
  }
  {
    typedef std::function<int(int, int, int)> Func;
    Func f = g3;
    Func& fr = (f = (Func &)f);
    assert(&fr == &f);
    assert(*f.target<int(*)(int, int, int)>() == g3);
  }
#if TEST_STD_VER >= 11
  assert(globalMemCounter.checkOutstandingNewEq(0));
  {
    std::function<int(int)> f = A();
    assert(A::count == 1);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(f.target<A>());
    assert(f.target<int (*)(int)>() == 0);
    std::function<int(int)> f2;
    f2 = std::move(f);
    assert(A::count == 1);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(f2.target<A>());
    assert(f2.target<int (*)(int)>() == 0);
    assert(f.target<A>() == 0);
    assert(f.target<int (*)(int)>() == 0);
  }
#endif
}
