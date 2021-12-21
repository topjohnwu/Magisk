// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <tuple>

// template <class TupleLike> tuple(TupleLike&&); // libc++ extension

// See llvm.org/PR31384
#include <tuple>
#include <cassert>

int count = 0;

struct Explicit {
  Explicit() = default;
  explicit Explicit(int) {}
};

struct Implicit {
  Implicit() = default;
  Implicit(int) {}
};

template<class T>
struct Derived : std::tuple<T> {
  using std::tuple<T>::tuple;
  template<class U>
  operator std::tuple<U>() && { ++count; return {}; }
};


template<class T>
struct ExplicitDerived : std::tuple<T> {
  using std::tuple<T>::tuple;
  template<class U>
  explicit operator std::tuple<U>() && { ++count; return {}; }
};

int main() {
  {
    std::tuple<Explicit> foo = Derived<int>{42}; ((void)foo);
    assert(count == 1);
    std::tuple<Explicit> bar(Derived<int>{42}); ((void)bar);
    assert(count == 2);
  }
  count = 0;
  {
    std::tuple<Implicit> foo = Derived<int>{42}; ((void)foo);
    assert(count == 1);
    std::tuple<Implicit> bar(Derived<int>{42}); ((void)bar);
    assert(count == 2);
  }
  count = 0;
  {
    static_assert(!std::is_convertible<
        ExplicitDerived<int>, std::tuple<Explicit>>::value, "");
    std::tuple<Explicit> bar(ExplicitDerived<int>{42}); ((void)bar);
    assert(count == 1);
  }
  count = 0;
  {
    // FIXME: Libc++ incorrectly rejects this code.
#ifndef _LIBCPP_VERSION
    std::tuple<Implicit> foo = ExplicitDerived<int>{42}; ((void)foo);
    static_assert(std::is_convertible<
        ExplicitDerived<int>, std::tuple<Implicit>>::value,
        "correct STLs accept this");
#else
    static_assert(!std::is_convertible<
        ExplicitDerived<int>, std::tuple<Implicit>>::value,
        "libc++ incorrectly rejects this");
#endif
    assert(count == 0);
    std::tuple<Implicit> bar(ExplicitDerived<int>{42}); ((void)bar);
    assert(count == 1);
  }
  count = 0;

}
