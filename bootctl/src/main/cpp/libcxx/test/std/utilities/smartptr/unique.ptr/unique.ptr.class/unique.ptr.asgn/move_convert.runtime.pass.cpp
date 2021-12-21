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

// Test unique_ptr converting move assignment

#include <memory>
#include <utility>
#include <cassert>

#include "unique_ptr_test_helper.h"

template <class APtr, class BPtr>
void testAssign(APtr& aptr, BPtr& bptr) {
  A* p = bptr.get();
  assert(A::count == 2);
  aptr = std::move(bptr);
  assert(aptr.get() == p);
  assert(bptr.get() == 0);
  assert(A::count == 1);
  assert(B::count == 1);
}

template <class LHS, class RHS>
void checkDeleter(LHS& lhs, RHS& rhs, int LHSState, int RHSState) {
  assert(lhs.get_deleter().state() == LHSState);
  assert(rhs.get_deleter().state() == RHSState);
}

template <class T>
struct NCConvertingDeleter {
  NCConvertingDeleter() = default;
  NCConvertingDeleter(NCConvertingDeleter const&) = delete;
  NCConvertingDeleter(NCConvertingDeleter&&) = default;

  template <class U>
  NCConvertingDeleter(NCConvertingDeleter<U>&&) {}

  void operator()(T*) const {}
};

template <class T>
struct NCConvertingDeleter<T[]> {
  NCConvertingDeleter() = default;
  NCConvertingDeleter(NCConvertingDeleter const&) = delete;
  NCConvertingDeleter(NCConvertingDeleter&&) = default;

  template <class U>
  NCConvertingDeleter(NCConvertingDeleter<U>&&) {}

  void operator()(T*) const {}
};

struct GenericDeleter {
  void operator()(void*) const;
};

struct NCGenericDeleter {
  NCGenericDeleter() = default;
  NCGenericDeleter(NCGenericDeleter const&) = delete;
  NCGenericDeleter(NCGenericDeleter&&) = default;

  void operator()(void*) const {}
};

void test_sfinae() {
  using DA = NCConvertingDeleter<A[]>;        // non-copyable deleters
  using DAC = NCConvertingDeleter<const A[]>; // non-copyable deleters

  using UA = std::unique_ptr<A[]>;
  using UAC = std::unique_ptr<const A[]>;
  using UAD = std::unique_ptr<A[], DA>;
  using UACD = std::unique_ptr<const A[], DAC>;

  { // cannot move from an lvalue
    static_assert(std::is_assignable<UAC, UA&&>::value, "");
    static_assert(!std::is_assignable<UAC, UA&>::value, "");
    static_assert(!std::is_assignable<UAC, const UA&>::value, "");
  }
  { // cannot move if the deleter-types cannot convert
    static_assert(std::is_assignable<UACD, UAD&&>::value, "");
    static_assert(!std::is_assignable<UACD, UAC&&>::value, "");
    static_assert(!std::is_assignable<UAC, UACD&&>::value, "");
  }
  { // cannot move-convert with reference deleters of different types
    using UA1 = std::unique_ptr<A[], DA&>;
    using UA2 = std::unique_ptr<A[], DAC&>;
    static_assert(!std::is_assignable<UA1, UA2&&>::value, "");
  }
  { // cannot move-convert with reference deleters of different types
    using UA1 = std::unique_ptr<A[], const DA&>;
    using UA2 = std::unique_ptr<A[], const DAC&>;
    static_assert(!std::is_assignable<UA1, UA2&&>::value, "");
  }
  { // cannot move-convert from unique_ptr<Single>
    using UA1 = std::unique_ptr<A[]>;
    using UA2 = std::unique_ptr<A>;
    static_assert(!std::is_assignable<UA1, UA2&&>::value, "");
  }
  { // cannot move-convert from unique_ptr<Array[]>
    using UA1 = std::unique_ptr<A[], NCGenericDeleter>;
    using UA2 = std::unique_ptr<A, NCGenericDeleter>;
    static_assert(!std::is_assignable<UA1, UA2&&>::value, "");
  }
}

int main() {
  test_sfinae();
  // FIXME: add tests
}
