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

// class path

// int compare(path const&) const noexcept;
// int compare(string_type const&) const;
// int compare(value_type const*) const;
//
// bool operator==(path const&, path const&) noexcept;
// bool operator!=(path const&, path const&) noexcept;
// bool operator< (path const&, path const&) noexcept;
// bool operator<=(path const&, path const&) noexcept;
// bool operator> (path const&, path const&) noexcept;
// bool operator>=(path const&, path const&) noexcept;
//
// size_t hash_value(path const&) noexcept;


#include "filesystem_include.hpp"
#include <type_traits>
#include <vector>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "filesystem_test_helper.hpp"
#include "verbose_assert.h"

struct PathCompareTest {
  const char* LHS;
  const char* RHS;
  int expect;
};

#define LONGA "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
#define LONGB "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
#define LONGC "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
#define LONGD "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
const PathCompareTest CompareTestCases[] =
{
    {"", "",  0},
    {"a", "", 1},
    {"", "a", -1},
    {"a/b/c", "a/b/c", 0},
    {"b/a/c", "a/b/c", 1},
    {"a/b/c", "b/a/c", -1},
    {"a/b", "a/b/c", -1},
    {"a/b/c", "a/b", 1},
    {"a/b/", "a/b/.", -1},
    {"a/b/", "a/b",    1},
    {"a/b//////", "a/b/////.", -1},
    {"a/.././b", "a///..//.////b", 0},
    {"//foo//bar///baz////", "//foo/bar/baz/", 0}, // duplicate separators
    {"///foo/bar", "/foo/bar", 0}, // "///" is not a root directory
    {"/foo/bar/", "/foo/bar", 1}, // trailing separator
    {"foo", "/foo", -1}, // if !this->has_root_directory() and p.has_root_directory(), a value less than 0.
    {"/foo", "foo", 1}, //  if this->has_root_directory() and !p.has_root_directory(), a value greater than 0.
    {"//" LONGA "////" LONGB "/" LONGC "///" LONGD, "//" LONGA "/" LONGB "/" LONGC "/" LONGD, 0},
    { LONGA "/" LONGB "/" LONGC, LONGA "/" LONGB "/" LONGB, 1}

};
#undef LONGA
#undef LONGB
#undef LONGC
#undef LONGD

static inline int normalize_ret(int ret)
{
  return ret < 0 ? -1 : (ret > 0 ? 1 : 0);
}

void test_compare_basic()
{
  using namespace fs;
  for (auto const & TC : CompareTestCases) {
    const path p1(TC.LHS);
    const path p2(TC.RHS);
    const std::string R(TC.RHS);
    const std::string_view RV(TC.RHS);
    const int E = TC.expect;
    { // compare(...) functions
      DisableAllocationGuard g; // none of these operations should allocate

      // check runtime results
      int ret1 = normalize_ret(p1.compare(p2));
      int ret2 = normalize_ret(p1.compare(R));
      int ret3 = normalize_ret(p1.compare(TC.RHS));
      int ret4 = normalize_ret(p1.compare(RV));

      g.release();
      ASSERT_EQ(ret1, ret2);
      ASSERT_EQ(ret1, ret3);
      ASSERT_EQ(ret1, ret4);
      ASSERT_EQ(ret1, E)
          << DISPLAY(TC.LHS) << DISPLAY(TC.RHS);

      // check signatures
      ASSERT_NOEXCEPT(p1.compare(p2));
    }
    { // comparison operators
      DisableAllocationGuard g; // none of these operations should allocate

      // Check runtime result
      assert((p1 == p2) == (E == 0));
      assert((p1 != p2) == (E != 0));
      assert((p1 <  p2) == (E <  0));
      assert((p1 <= p2) == (E <= 0));
      assert((p1 >  p2) == (E >  0));
      assert((p1 >= p2) == (E >= 0));

      // Check signatures
      ASSERT_NOEXCEPT(p1 == p2);
      ASSERT_NOEXCEPT(p1 != p2);
      ASSERT_NOEXCEPT(p1 <  p2);
      ASSERT_NOEXCEPT(p1 <= p2);
      ASSERT_NOEXCEPT(p1 >  p2);
      ASSERT_NOEXCEPT(p1 >= p2);
    }
    { // check hash values
      auto h1 = hash_value(p1);
      auto h2 = hash_value(p2);
      assert((h1 == h2) == (p1 == p2));
      // check signature
      ASSERT_SAME_TYPE(size_t, decltype(hash_value(p1)));
      ASSERT_NOEXCEPT(hash_value(p1));
    }
  }
}

int CompareElements(std::vector<std::string> const& LHS, std::vector<std::string> const& RHS) {
  bool IsLess = std::lexicographical_compare(LHS.begin(), LHS.end(), RHS.begin(), RHS.end());
  if (IsLess)
    return -1;

  bool IsGreater = std::lexicographical_compare(RHS.begin(), RHS.end(), LHS.begin(), LHS.end());
  if (IsGreater)
    return 1;

  return 0;
}

void test_compare_elements() {
  struct {
    std::vector<std::string> LHSElements;
    std::vector<std::string> RHSElements;
    int Expect;
  } TestCases[] = {
      {{"a"}, {"a"}, 0},
      {{"a"}, {"b"}, -1},
      {{"b"}, {"a"}, 1},
      {{"a", "b", "c"}, {"a", "b", "c"}, 0},
      {{"a", "b", "c"}, {"a", "b", "d"}, -1},
      {{"a", "b", "d"}, {"a", "b", "c"}, 1},
      {{"a", "b"}, {"a", "b", "c"}, -1},
      {{"a", "b", "c"}, {"a", "b"}, 1},

  };

  auto BuildPath = [](std::vector<std::string> const& Elems) {
    fs::path p;
    for (auto &E : Elems)
      p /= E;
    return p;
  };

  for (auto &TC : TestCases) {
    fs::path LHS = BuildPath(TC.LHSElements);
    fs::path RHS = BuildPath(TC.RHSElements);
    const int ExpectCmp = CompareElements(TC.LHSElements, TC.RHSElements);
    assert(ExpectCmp == TC.Expect);
    const int GotCmp = normalize_ret(LHS.compare(RHS));
    assert(GotCmp == TC.Expect);
  }
}

int main() {
  test_compare_basic();
  test_compare_elements();
}
