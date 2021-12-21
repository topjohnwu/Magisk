//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <filesystem>

// class directory_entry

// bool operator==(directory_entry const&) const noexcept;
// bool operator!=(directory_entry const&) const noexcept;
// bool operator< (directory_entry const&) const noexcept;
// bool operator<=(directory_entry const&) const noexcept;
// bool operator> (directory_entry const&) const noexcept;
// bool operator>=(directory_entry const&) const noexcept;


#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>


#define CHECK_OP(Op) \
  static_assert(std::is_same<decltype(ce. operator Op (ce)), bool>::value, ""); \
  static_assert(noexcept(ce.operator Op (ce)), "Operation must be noexcept" )

void test_comparison_signatures() {
  using namespace fs;
  path const p("foo/bar/baz");
  // Check that the operators are member functions with the correct signatures.
  {
    directory_entry const ce(p);
    CHECK_OP(==);
    CHECK_OP(!=);
    CHECK_OP(< );
    CHECK_OP(<=);
    CHECK_OP(> );
    CHECK_OP(>=);
  }
}
#undef CHECK_OP

// The actual semantics of the comparisons are testing via paths operators.
void test_comparisons_simple() {
  using namespace fs;
  typedef std::pair<path, path> TestType;
  TestType TestCases[] =
  {
      {"", ""},
      {"", "a"},
      {"a", "a"},
      {"a", "b"},
      {"foo/bar/baz", "foo/bar/baz/"}
  };
  auto TestFn = [](path const& LHS, const directory_entry& LHSE,
                   path const& RHS, const directory_entry& RHSE) {
    assert((LHS == RHS) == (LHSE == RHSE));
    assert((LHS != RHS) == (LHSE != RHSE));
    assert((LHS < RHS) ==  (LHSE < RHSE));
    assert((LHS <= RHS) == (LHSE <= RHSE));
    assert((LHS > RHS) ==  (LHSE > RHSE));
    assert((LHS >= RHS) == (LHSE >= RHSE));
  };
  for (auto const& TC : TestCases) {
    const directory_entry L(TC.first);
    const directory_entry R(TC.second);
    TestFn(TC.first,  L, TC.second, R);
    TestFn(TC.second, R, TC.first, L);
  }
}

int main() {
  test_comparison_signatures();
  test_comparisons_simple();
}
