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

// test reset

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

template <bool IsArray>
void test_reset_pointer() {
  typedef typename std::conditional<IsArray, A[], A>::type VT;
  const int expect_alive = IsArray ? 3 : 1;
#if TEST_STD_VER >= 11
  {
    using U = std::unique_ptr<VT>;
    U u; ((void)u);
    ASSERT_NOEXCEPT(u.reset((A*)nullptr));
  }
#endif
  {
    std::unique_ptr<VT> p(newValue<VT>(expect_alive));
    assert(A::count == expect_alive);
    A* i = p.get();
    assert(i != nullptr);
    A* new_value = newValue<VT>(expect_alive);
    assert(A::count == (expect_alive * 2));
    p.reset(new_value);
    assert(A::count == expect_alive);
    assert(p.get() == new_value);
  }
  assert(A::count == 0);
  {
    std::unique_ptr<const VT> p(newValue<const VT>(expect_alive));
    assert(A::count == expect_alive);
    const A* i = p.get();
    assert(i != nullptr);
    A* new_value = newValue<VT>(expect_alive);
    assert(A::count == (expect_alive * 2));
    p.reset(new_value);
    assert(A::count == expect_alive);
    assert(p.get() == new_value);
  }
  assert(A::count == 0);
}

template <bool IsArray>
void test_reset_nullptr() {
  typedef typename std::conditional<IsArray, A[], A>::type VT;
  const int expect_alive = IsArray ? 3 : 1;
#if TEST_STD_VER >= 11
  {
    using U = std::unique_ptr<VT>;
    U u; ((void)u);
    ASSERT_NOEXCEPT(u.reset(nullptr));
  }
#endif
  {
    std::unique_ptr<VT> p(newValue<VT>(expect_alive));
    assert(A::count == expect_alive);
    A* i = p.get();
    assert(i != nullptr);
    p.reset(nullptr);
    assert(A::count == 0);
    assert(p.get() == nullptr);
  }
  assert(A::count == 0);
}


template <bool IsArray>
void test_reset_no_arg() {
  typedef typename std::conditional<IsArray, A[], A>::type VT;
  const int expect_alive = IsArray ? 3 : 1;
#if TEST_STD_VER >= 11
  {
    using U = std::unique_ptr<VT>;
    U u; ((void)u);
    ASSERT_NOEXCEPT(u.reset());
  }
#endif
  {
    std::unique_ptr<VT> p(newValue<VT>(expect_alive));
    assert(A::count == expect_alive);
    A* i = p.get();
    assert(i != nullptr);
    p.reset();
    assert(A::count == 0);
    assert(p.get() == nullptr);
  }
  assert(A::count == 0);
}

int main() {
  {
    test_reset_pointer</*IsArray*/ false>();
    test_reset_nullptr<false>();
    test_reset_no_arg<false>();
  }
  {
    test_reset_pointer</*IsArray*/true>();
    test_reset_nullptr<true>();
    test_reset_no_arg<true>();
  }
}
