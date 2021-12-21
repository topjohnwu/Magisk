//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-exceptions
// Test asan vector annotations with a class that throws in a CTOR.

#include <vector>
#include <cassert>

#include "test_macros.h"
#include "asan_testing.h"

class X {
public:
  X(const X &x) { Init(x.a); }
  X(char arg) { Init(arg); }
  X() { Init(42); }
  X &operator=(const X &x) {
    Init(x.a);
    return *this;
  }
  void Init(char arg) {
    if (arg == 42)
      throw 0;
    if (arg == 66)
      arg = 42;
    a = arg;
  }
  char get() const { return a; }
  void set(char arg) { a = arg; }

private:
  char a;
};

class ThrowOnCopy {
public:
    ThrowOnCopy() : should_throw(false) {}
    explicit ThrowOnCopy(bool xshould_throw) : should_throw(xshould_throw) {}

    ThrowOnCopy(ThrowOnCopy const & other)
        : should_throw(other.should_throw)
    {
        if (should_throw) {
            throw 0;
        }
    }

    bool should_throw;
};

void test_push_back() {
  std::vector<X> v;
  v.reserve(2);
  v.push_back(X(2));
  assert(v.size() == 1);
  try {
    v.push_back(X(66));
    assert(0);
  } catch (int e) {
    assert(v.size() == 1);
  }
  assert(v.size() == 1);
  assert(is_contiguous_container_asan_correct(v));
}

void test_emplace_back() {
#if TEST_STD_VER >= 11
  std::vector<X> v;
  v.reserve(2);
  v.push_back(X(2));
  assert(v.size() == 1);
  try {
    v.emplace_back(42);
    assert(0);
  } catch (int e) {
    assert(v.size() == 1);
  }
  assert(v.size() == 1);
  assert(is_contiguous_container_asan_correct(v));
#endif
}

void test_insert_range() {
  std::vector<X> v;
  v.reserve(4);
  v.push_back(X(1));
  v.push_back(X(2));
  assert(v.size() == 2);
  assert(v.capacity() >= 4);
  try {
    char a[2] = {21, 42};
    v.insert(v.end(), a, a + 2);
    assert(0);
  } catch (int e) {
    assert(v.size() == 3);
  }
  assert(v.size() == 3);
  assert(is_contiguous_container_asan_correct(v));
}

void test_insert() {
  std::vector<X> v;
  v.reserve(3);
  v.insert(v.end(), X(1));
  v.insert(v.begin(), X(2));
  assert(v.size() == 2);
  try {
    v.insert(v.end(), X(66));
    assert(0);
  } catch (int e) {
    assert(v.size() == 2);
  }
  assert(v.size() == 2);
  assert(is_contiguous_container_asan_correct(v));
}

void test_emplace() {
#if TEST_STD_VER >= 11
  std::vector<X> v;
  v.reserve(3);
  v.insert(v.end(), X(1));
  v.insert(v.begin(), X(2));
  assert(v.size() == 2);
  try {
    v.emplace(v.end(), 42);
    assert(0);
  } catch (int e) {
    assert(v.size() == 2);
  }
  assert(v.size() == 2);
  assert(is_contiguous_container_asan_correct(v));
#endif
}

void test_insert_range2() {
  std::vector<X> v;
  v.reserve(4);
  v.insert(v.end(), X(1));
  v.insert(v.begin(), X(2));
  assert(v.size() == 2);
  assert(v.capacity() >= 4);
  try {
    char a[2] = {10, 42};
    v.insert(v.begin(), a, a + 2);
    assert(0);
  } catch (int e) {
    assert(v.size() <= 4);
    assert(is_contiguous_container_asan_correct(v));
    return;
  }
  assert(0);
}

void test_insert_n() {
  std::vector<X> v;
  v.reserve(10);
  v.insert(v.end(), X(1));
  v.insert(v.begin(), X(2));
  assert(v.size() == 2);
  try {
    v.insert(v.begin(), 1, X(66));
    assert(0);
  } catch (int e) {
    assert(v.size() <= 3);
    assert(is_contiguous_container_asan_correct(v));
    return;
  }
  assert(0);
}


void test_insert_n2() {
  std::vector<ThrowOnCopy> v(10);
  v.reserve(100);
  assert(v.size() == 10);
  v[6].should_throw = true;
  try {
    v.insert(v.cbegin(), 5, ThrowOnCopy());
    assert(0);
  } catch (int e) {
    assert(v.size() == 11);
    assert(is_contiguous_container_asan_correct(v));
    return;
  }
  assert(0);
}

void test_resize() {
  std::vector<X> v;
  v.reserve(3);
  v.push_back(X(0));
  try {
    v.resize(3);
    assert(0);
  } catch (int e) {
    assert(v.size() == 1);
  }
  assert(v.size() == 1);
  assert(is_contiguous_container_asan_correct(v));
}

void test_resize_param() {
  std::vector<X> v;
  v.reserve(3);
  v.push_back(X(0));
  try {
    v.resize(3, X(66));
    assert(0);
  } catch (int e) {
    assert(v.size() == 1);
  }
  assert(v.size() == 1);
  assert(is_contiguous_container_asan_correct(v));
}

int main() {
  test_push_back();
  test_emplace_back();
  test_insert_range();
  test_insert();
  test_emplace();
  test_insert_range2();
  test_insert_n();
  test_insert_n2();
  test_resize();
  test_resize_param();
}
