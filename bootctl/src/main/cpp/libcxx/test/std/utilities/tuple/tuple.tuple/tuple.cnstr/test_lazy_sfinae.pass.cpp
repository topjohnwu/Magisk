//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <utility>
#include <cassert>

template <class ConstructFrom>
struct ConstructibleFromT {
  ConstructibleFromT() = default;
  ConstructibleFromT(ConstructFrom v) : value(v) {}
  ConstructFrom value;
};

template <class AssertOn>
struct CtorAssertsT {
  bool defaulted;
  CtorAssertsT() : defaulted(true) {}
  template <class T>
  constexpr CtorAssertsT(T) : defaulted(false) {
      static_assert(!std::is_same<T, AssertOn>::value, "");
  }
};

template <class AllowT, class AssertT>
struct AllowAssertT {
  AllowAssertT() = default;
  AllowAssertT(AllowT) {}
  template <class U>
  constexpr AllowAssertT(U) {
      static_assert(!std::is_same<U, AssertT>::value, "");
  }
};

// Construct a tuple<T1, T2> from pair<int, int> where T1 and T2
// are not constructible from ints but T1 is constructible from std::pair.
// This considers the following constructors:
// (1) tuple(TupleLike) -> checks is_constructible<Tn, int>
// (2) tuple(UTypes...) -> checks is_constructible<T1, pair<int, int>>
//                            and is_default_constructible<T2>
// The point of this test is to ensure that the consideration of (1)
// short circuits before evaluating is_constructible<T2, int>, which
// will cause a static assertion.
void test_tuple_like_lazy_sfinae() {
#if defined(_LIBCPP_VERSION)
    // This test requires libc++'s reduced arity initialization.
    using T1 = ConstructibleFromT<std::pair<int, int>>;
    using T2 = CtorAssertsT<int>;
    std::pair<int, int> p(42, 100);
    std::tuple<T1, T2> t(p);
    assert(std::get<0>(t).value == p);
    assert(std::get<1>(t).defaulted);
#endif
}


struct NonConstCopyable {
  NonConstCopyable() = default;
  explicit NonConstCopyable(int v) : value(v) {}
  NonConstCopyable(NonConstCopyable&) = default;
  NonConstCopyable(NonConstCopyable const&) = delete;
  int value;
};

template <class T>
struct BlowsUpOnConstCopy {
  BlowsUpOnConstCopy() = default;
  constexpr BlowsUpOnConstCopy(BlowsUpOnConstCopy const&) {
      static_assert(!std::is_same<T, T>::value, "");
  }
  BlowsUpOnConstCopy(BlowsUpOnConstCopy&) = default;
};

// Test the following constructors:
// (1) tuple(Types const&...)
// (2) tuple(UTypes&&...)
// Test that (1) short circuits before evaluating the copy constructor of the
// second argument. Constructor (2) should be selected.
void test_const_Types_lazy_sfinae()
{
    NonConstCopyable v(42);
    BlowsUpOnConstCopy<int> b;
    std::tuple<NonConstCopyable, BlowsUpOnConstCopy<int>> t(v, b);
    assert(std::get<0>(t).value == 42);
}

int main() {
    test_tuple_like_lazy_sfinae();
    test_const_Types_lazy_sfinae();
}
