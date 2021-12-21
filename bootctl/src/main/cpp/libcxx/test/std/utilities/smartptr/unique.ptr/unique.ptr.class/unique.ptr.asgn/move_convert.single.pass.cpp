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

#include "deleter_types.h"
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

struct NCGenericDeleter {
  NCGenericDeleter() = default;
  NCGenericDeleter(NCGenericDeleter const&) = delete;
  NCGenericDeleter(NCGenericDeleter&&) = default;

  void operator()(void*) const {}
};

void test_sfinae() {
  using DA = NCConvertingDeleter<A>; // non-copyable deleters
  using DB = NCConvertingDeleter<B>;
  using UA = std::unique_ptr<A>;
  using UB = std::unique_ptr<B>;
  using UAD = std::unique_ptr<A, DA>;
  using UBD = std::unique_ptr<B, DB>;
  { // cannot move from an lvalue
    static_assert(std::is_assignable<UA, UB&&>::value, "");
    static_assert(!std::is_assignable<UA, UB&>::value, "");
    static_assert(!std::is_assignable<UA, const UB&>::value, "");
  }
  { // cannot move if the deleter-types cannot convert
    static_assert(std::is_assignable<UAD, UBD&&>::value, "");
    static_assert(!std::is_assignable<UAD, UB&&>::value, "");
    static_assert(!std::is_assignable<UA, UBD&&>::value, "");
  }
  { // cannot move-convert with reference deleters of different types
    using UA1 = std::unique_ptr<A, DA&>;
    using UB1 = std::unique_ptr<B, DB&>;
    static_assert(!std::is_assignable<UA1, UB1&&>::value, "");
  }
  { // cannot move-convert with reference deleters of different types
    using UA1 = std::unique_ptr<A, const DA&>;
    using UB1 = std::unique_ptr<B, const DB&>;
    static_assert(!std::is_assignable<UA1, UB1&&>::value, "");
  }
  { // cannot move-convert from unique_ptr<Array[]>
    using UA1 = std::unique_ptr<A>;
    using UA2 = std::unique_ptr<A[]>;
    using UB1 = std::unique_ptr<B[]>;
    static_assert(!std::is_assignable<UA1, UA2&&>::value, "");
    static_assert(!std::is_assignable<UA1, UB1&&>::value, "");
  }
  { // cannot move-convert from unique_ptr<Array[]>
    using UA1 = std::unique_ptr<A, NCGenericDeleter>;
    using UA2 = std::unique_ptr<A[], NCGenericDeleter>;
    using UB1 = std::unique_ptr<B[], NCGenericDeleter>;
    static_assert(!std::is_assignable<UA1, UA2&&>::value, "");
    static_assert(!std::is_assignable<UA1, UB1&&>::value, "");
  }
}

int main() {
  test_sfinae();
  {
    std::unique_ptr<B> bptr(new B);
    std::unique_ptr<A> aptr(new A);
    testAssign(aptr, bptr);
  }
  assert(A::count == 0);
  assert(B::count == 0);
  {
    Deleter<B> del(42);
    std::unique_ptr<B, Deleter<B> > bptr(new B, std::move(del));
    std::unique_ptr<A, Deleter<A> > aptr(new A);
    testAssign(aptr, bptr);
    checkDeleter(aptr, bptr, 42, 0);
  }
  assert(A::count == 0);
  assert(B::count == 0);
  {
    CDeleter<A> adel(6);
    CDeleter<B> bdel(42);
    std::unique_ptr<B, CDeleter<B>&> bptr(new B, bdel);
    std::unique_ptr<A, CDeleter<A>&> aptr(new A, adel);
    testAssign(aptr, bptr);
    checkDeleter(aptr, bptr, 42, 42);
  }
  assert(A::count == 0);
  assert(B::count == 0);
}
