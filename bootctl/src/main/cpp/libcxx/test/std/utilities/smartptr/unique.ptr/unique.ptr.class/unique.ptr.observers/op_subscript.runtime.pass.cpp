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

// test op[](size_t)

#include <memory>
#include <cassert>

class A {
  int state_;
  static int next_;

public:
  A() : state_(++next_) {}
  int get() const { return state_; }

  friend bool operator==(const A& x, int y) { return x.state_ == y; }

  A& operator=(int i) {
    state_ = i;
    return *this;
  }
};

int A::next_ = 0;

int main() {
  std::unique_ptr<A[]> p(new A[3]);
  assert(p[0] == 1);
  assert(p[1] == 2);
  assert(p[2] == 3);
  p[0] = 3;
  p[1] = 2;
  p[2] = 1;
  assert(p[0] == 3);
  assert(p[1] == 2);
  assert(p[2] == 1);
}
