//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// <optional>

// template <class T, class U> constexpr bool operator>=(const optional<T>& x, const U& v);
// template <class T, class U> constexpr bool operator>=(const U& v, const optional<T>& x);

#include <optional>

using std::optional;

struct X {
  int i_;

  constexpr X(int i) : i_(i) {}
};

constexpr bool operator>=(const X& lhs, const X& rhs) {
  return lhs.i_ >= rhs.i_;
}

int main() {
  {
    typedef X T;
    typedef optional<T> O;

    constexpr T val(2);
    constexpr O o1;      // disengaged
    constexpr O o2{1};   // engaged
    constexpr O o3{val}; // engaged

    static_assert(!(o1 >= T(1)), "");
    static_assert((o2 >= T(1)), ""); // equal
    static_assert((o3 >= T(1)), "");
    static_assert(!(o2 >= val), "");
    static_assert((o3 >= val), ""); // equal
    static_assert(!(o3 >= T(3)), "");

    static_assert((T(1) >= o1), "");
    static_assert((T(1) >= o2), ""); // equal
    static_assert(!(T(1) >= o3), "");
    static_assert((val >= o2), "");
    static_assert((val >= o3), ""); // equal
    static_assert((T(3) >= o3), "");
  }
  {
    using O = optional<int>;
    constexpr O o1(42);
    static_assert(o1 >= 42l, "");
    static_assert(!(11l >= o1), "");
  }
  {
    using O = optional<const int>;
    constexpr O o1(42);
    static_assert(o1 >= 42, "");
    static_assert(!(11 >= o1), "");
  }
}
