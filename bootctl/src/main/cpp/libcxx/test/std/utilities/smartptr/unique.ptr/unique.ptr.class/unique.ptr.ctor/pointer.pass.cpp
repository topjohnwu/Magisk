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

//=============================================================================
// TESTING std::unique_ptr::unique_ptr(pointer)
//
// Concerns:
//   1 The pointer constructor works for any default constructible deleter types.
//   2 The pointer constructor accepts pointers to derived types.
//   2 The stored type 'T' is allowed to be incomplete.
//
// Plan
//  1 Construct unique_ptr<T, D>'s with a pointer to 'T' and various deleter
//   types (C-1)
//  2 Construct unique_ptr<T, D>'s with a pointer to 'D' and various deleter
//    types where 'D' is derived from 'T'. (C-1,2)
//  3 Construct a unique_ptr<T, D> with a pointer to 'T' and various deleter
//    types where 'T' is an incomplete type (C-1,3)

// Test unique_ptr(pointer) ctor

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"

// unique_ptr(pointer) ctor should only require default Deleter ctor

template <bool IsArray>
void test_pointer() {
  typedef typename std::conditional<!IsArray, A, A[]>::type ValueT;
  const int expect_alive = IsArray ? 5 : 1;
#if TEST_STD_VER >= 11
  {
    using U1 = std::unique_ptr<ValueT>;
    using U2 = std::unique_ptr<ValueT, Deleter<ValueT> >;

    // Test for noexcept
    static_assert(std::is_nothrow_constructible<U1, A*>::value, "");
    static_assert(std::is_nothrow_constructible<U2, A*>::value, "");

    // Test for explicit
    static_assert(!std::is_convertible<A*, U1>::value, "");
    static_assert(!std::is_convertible<A*, U2>::value, "");
  }
#endif
  {
    A* p = newValue<ValueT>(expect_alive);
    assert(A::count == expect_alive);
    std::unique_ptr<ValueT> s(p);
    assert(s.get() == p);
  }
  assert(A::count == 0);
  {
    A* p = newValue<ValueT>(expect_alive);
    assert(A::count == expect_alive);
    std::unique_ptr<ValueT, NCDeleter<ValueT> > s(p);
    assert(s.get() == p);
    assert(s.get_deleter().state() == 0);
  }
  assert(A::count == 0);
}

void test_derived() {
  {
    B* p = new B;
    assert(A::count == 1);
    assert(B::count == 1);
    std::unique_ptr<A> s(p);
    assert(s.get() == p);
  }
  assert(A::count == 0);
  assert(B::count == 0);
  {
    B* p = new B;
    assert(A::count == 1);
    assert(B::count == 1);
    std::unique_ptr<A, NCDeleter<A> > s(p);
    assert(s.get() == p);
    assert(s.get_deleter().state() == 0);
  }
  assert(A::count == 0);
  assert(B::count == 0);
}

#if TEST_STD_VER >= 11
struct NonDefaultDeleter {
  NonDefaultDeleter() = delete;
  void operator()(void*) const {}
};

struct GenericDeleter {
  void operator()(void*) const;
};
#endif

template <class T>
void test_sfinae() {
#if TEST_STD_VER >= 11
  { // the constructor does not participate in overload resultion when
    // the deleter is a pointer type
    using U = std::unique_ptr<T, void (*)(void*)>;
    static_assert(!std::is_constructible<U, T*>::value, "");
  }
  { // the constructor does not participate in overload resolution when
    // the deleter is not default constructible
    using Del = CDeleter<T>;
    using U1 = std::unique_ptr<T, NonDefaultDeleter>;
    using U2 = std::unique_ptr<T, Del&>;
    using U3 = std::unique_ptr<T, Del const&>;
    static_assert(!std::is_constructible<U1, T*>::value, "");
    static_assert(!std::is_constructible<U2, T*>::value, "");
    static_assert(!std::is_constructible<U3, T*>::value, "");
  }
#endif
}

static void test_sfinae_runtime() {
#if TEST_STD_VER >= 11
  { // the constructor does not participate in overload resolution when
    // a base <-> derived conversion would occur.
    using UA = std::unique_ptr<A[]>;
    using UAD = std::unique_ptr<A[], GenericDeleter>;
    using UAC = std::unique_ptr<const A[]>;
    using UB = std::unique_ptr<B[]>;
    using UBD = std::unique_ptr<B[], GenericDeleter>;
    using UBC = std::unique_ptr<const B[]>;

    static_assert(!std::is_constructible<UA, B*>::value, "");
    static_assert(!std::is_constructible<UB, A*>::value, "");
    static_assert(!std::is_constructible<UAD, B*>::value, "");
    static_assert(!std::is_constructible<UBD, A*>::value, "");
    static_assert(!std::is_constructible<UAC, const B*>::value, "");
    static_assert(!std::is_constructible<UBC, const A*>::value, "");
  }
#endif
}

DEFINE_AND_RUN_IS_INCOMPLETE_TEST({
  { doIncompleteTypeTest(1, getNewIncomplete()); }
  checkNumIncompleteTypeAlive(0);
  {
    doIncompleteTypeTest<IncompleteType, NCDeleter<IncompleteType> >(
        1, getNewIncomplete());
  }
  checkNumIncompleteTypeAlive(0);
})

int main() {
  {
    test_pointer</*IsArray*/ false>();
    test_derived();
    test_sfinae<int>();
  }
  {
    test_pointer</*IsArray*/ true>();
    test_sfinae<int[]>();
    test_sfinae_runtime();
  }
}
