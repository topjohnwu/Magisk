// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// See https://bugs.llvm.org/show_bug.cgi?id=33271
// UNSUPPORTED: ubsan

#include <experimental/coroutine>
#include <vector>
#include <cassert>

#include "coroutine_types.h"

using namespace std::experimental;

struct minig {
  struct promise_type {
    int current_value;
    suspend_always yield_value(int value) {
      this->current_value = value;
      return {};
    }
    suspend_always initial_suspend() { return {}; }
    suspend_always final_suspend() { return {}; }
    minig get_return_object() { return minig{this}; };
    void return_void() {}
    void unhandled_exception() {}
  };

  bool move_next() {
    p.resume();
    return !p.done();
  }
  int current_value() { return p.promise().current_value; }

  minig(minig &&rhs) : p(rhs.p) { rhs.p = nullptr; }

  ~minig() {
    if (p)
      p.destroy();
  }

private:
  explicit minig(promise_type *p)
      : p(coroutine_handle<promise_type>::from_promise(*p)) {}

  coroutine_handle<promise_type> p;
};


minig mini_count(int n) {
  for (int i = 0; i < n; i++) {
    co_yield i;
  }
}

generator<int> count(int n) {
  for (int i = 0; i < n; ++i)
    co_yield i;
}

generator<int> range(int from, int n) {
  for (int i = from; i < n; ++i)
    co_yield i;
}

void test_count() {
  const std::vector<int> expect = {0, 1, 2, 3, 4};
  std::vector<int> got;
  for (auto x : count(5))
    got.push_back(x);
  assert(expect == got);
}

void test_range() {
  int sum = 0;
   for (auto v: range(1, 20))
      sum += v;
   assert(sum == 190);
}

void test_mini_generator() {
  int sum = 0;
  auto g = mini_count(5);
  while (g.move_next()) {
     sum += g.current_value();
  }
  assert(sum == 10);
}

int main() {
  test_count();
  test_range();
  test_mini_generator();
}
