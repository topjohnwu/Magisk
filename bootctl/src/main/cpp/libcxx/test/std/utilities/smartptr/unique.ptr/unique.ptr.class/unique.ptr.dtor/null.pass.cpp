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

// The deleter is not called if get() == 0

#include <memory>
#include <cassert>

class Deleter {
  int state_;

  Deleter(Deleter&);
  Deleter& operator=(Deleter&);

public:
  Deleter() : state_(0) {}

  int state() const { return state_; }

  void operator()(void*) { ++state_; }
};

template <class T>
void test_basic() {
  Deleter d;
  assert(d.state() == 0);
  {
    std::unique_ptr<T, Deleter&> p(nullptr, d);
    assert(p.get() == nullptr);
    assert(&p.get_deleter() == &d);
  }
  assert(d.state() == 0);
}

int main() {
  test_basic<int>();
  test_basic<int[]>();
}
