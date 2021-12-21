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

// template <class... Types>
//   struct tuple_size<tuple<Types...>>
//     : public integral_constant<size_t, sizeof...(Types)> { };

// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-structured-bindings

#include <tuple>
#include <array>
#include <type_traits>
#include <cassert>

struct S { int x; };

void test_decomp_user_type() {
  {
    S s{99};
    auto [m1] = s;
    auto& [r1] = s;
    assert(m1 == 99);
    assert(&r1 == &s.x);
  }
  {
    S const s{99};
    auto [m1] = s;
    auto& [r1] = s;
    assert(m1 == 99);
    assert(&r1 == &s.x);
  }
}

void test_decomp_tuple() {
  typedef std::tuple<int> T;
  {
    T s{99};
    auto [m1] = s;
    auto& [r1] = s;
    assert(m1 == 99);
    assert(&r1 == &std::get<0>(s));
  }
  {
    T const s{99};
    auto [m1] = s;
    auto& [r1] = s;
    assert(m1 == 99);
    assert(&r1 == &std::get<0>(s));
  }
}


void test_decomp_pair() {
  typedef std::pair<int, double> T;
  {
    T s{99, 42.5};
    auto [m1, m2] = s;
    auto& [r1, r2] = s;
    assert(m1 == 99);
    assert(m2 == 42.5);
    assert(&r1 == &std::get<0>(s));
    assert(&r2 == &std::get<1>(s));
  }
  {
    T const s{99, 42.5};
    auto [m1, m2] = s;
    auto& [r1, r2] = s;
    assert(m1 == 99);
    assert(m2 == 42.5);
    assert(&r1 == &std::get<0>(s));
    assert(&r2 == &std::get<1>(s));
  }
}

void test_decomp_array() {
  typedef std::array<int, 3> T;
  {
    T s{{99, 42, -1}};
    auto [m1, m2, m3] = s;
    auto& [r1, r2, r3] = s;
    assert(m1 == 99);
    assert(m2 == 42);
    assert(m3 == -1);
    assert(&r1 == &std::get<0>(s));
    assert(&r2 == &std::get<1>(s));
    assert(&r3 == &std::get<2>(s));
  }
  {
    T const s{{99, 42, -1}};
    auto [m1, m2, m3] = s;
    auto& [r1, r2, r3] = s;
    assert(m1 == 99);
    assert(m2 == 42);
    assert(m3 == -1);
    assert(&r1 == &std::get<0>(s));
    assert(&r2 == &std::get<1>(s));
    assert(&r3 == &std::get<2>(s));
  }
}

struct Test {
  int x;
};

template <size_t N>
int get(Test const&) { static_assert(N == 0, ""); return -1; }

template <>
class std::tuple_element<0, Test> {
public:
  typedef int type;
};

void test_before_tuple_size_specialization() {
  Test const t{99};
  auto& [p] = t;
  assert(p == 99);
}

template <>
struct std::tuple_size<Test> {
public:
  static const size_t value = 1;
};

void test_after_tuple_size_specialization() {
  Test const t{99};
  auto& [p] = t;
  assert(p == -1);
}

int main() {
  test_decomp_user_type();
  test_decomp_tuple();
  test_decomp_pair();
  test_decomp_array();
  test_before_tuple_size_specialization();
  test_after_tuple_size_specialization();
}
