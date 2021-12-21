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

// path& operator+=(const path& x);
// path& operator+=(const string_type& x);
// path& operator+=(string_view x);
// path& operator+=(const value_type* x);
// path& operator+=(value_type x);
// template <class Source>
//   path& operator+=(const Source& x);
// template <class EcharT>
//   path& operator+=(EcharT x);
// template <class Source>
//   path& concat(const Source& x);
// template <class InputIterator>
//   path& concat(InputIterator first, InputIterator last);


#include "filesystem_include.hpp"
#include <type_traits>
#include <string>
#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "filesystem_test_helper.hpp"


struct ConcatOperatorTestcase {
  MultiStringType lhs;
  MultiStringType rhs;
  MultiStringType expect;
};

#define LONGSTR "LONGSTR_LONGSTR_LONGSTR_LONGSTR_LONGSTR_LONGSTR_LONGSTR_LONGSTR_LONGSTR_LONGSTR_LONGSTR_LONGSTR"
#define S(Str) MKSTR(Str)
const ConcatOperatorTestcase Cases[] =
    {
        {S(""),         S(""),                  S("")}
      , {S("p1"),       S("p2"),                S("p1p2")}
      , {S("p1/"),      S("/p2"),               S("p1//p2")}
      , {S(""),         S("\\foo/bar/baz"),     S("\\foo/bar/baz")}
      , {S("c:\\foo"),  S(""),                  S("c:\\foo")}
      , {S(LONGSTR),    S("foo"),               S(LONGSTR "foo")}
      , {S("abcdefghijklmnopqrstuvwxyz/\\"), S("/\\123456789"), S("abcdefghijklmnopqrstuvwxyz/\\/\\123456789")}
    };
const ConcatOperatorTestcase LongLHSCases[] =
    {
        {S(""),        S(LONGSTR),     S(LONGSTR)}
      , {S("p1/"),     S(LONGSTR),      S("p1/" LONGSTR)}
    };
const ConcatOperatorTestcase CharTestCases[] =
    {
        {S(""),       S("P"), S("P")}
      , {S("/fooba"), S("r"), S("/foobar")}
    };
#undef S
#undef LONGSTR

// The concat operator may need to allocate a temporary buffer before a code_cvt
// conversion. Test if this allocation occurs by:
//   1. Create a path, `LHS`, and reserve enough space to append `RHS`.
//      This prevents `LHS` from allocating during the actual appending.
//   2. Create a `Source` object `RHS`, which represents a "large" string.
//      (The string must not trigger the SSO)
//   3. Concat `RHS` to `LHS` and check for the expected allocation behavior.
template <class CharT>
void doConcatSourceAllocTest(ConcatOperatorTestcase const& TC)
{
  using namespace fs;
  using Ptr = CharT const*;
  using Str = std::basic_string<CharT>;
  using StrView = std::basic_string_view<CharT>;
  using InputIter = input_iterator<Ptr>;

  const Ptr L = TC.lhs;
  const Ptr R = TC.rhs;
  const Ptr E =  TC.expect;
  std::size_t ReserveSize = StrLen(E) + 1;
  // basic_string
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    Str  RHS(R);
    {
      DisableAllocationGuard g;
      LHS += RHS;
    }
    assert(LHS == E);
  }
  // basic_string_view
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    StrView  RHS(R);
    {
      DisableAllocationGuard g;
      LHS += RHS;
    }
    assert(LHS == E);
  }
  // CharT*
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    Ptr RHS(R);
    {
      DisableAllocationGuard g;
      LHS += RHS;
    }
    assert(LHS == E);
  }
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    Ptr RHS(R);
    {
      DisableAllocationGuard g;
      LHS.concat(RHS, StrEnd(RHS));
    }
    assert(LHS == E);
  }
  // input iterator - For non-native char types, appends needs to copy the
  // iterator range into a contiguous block of memory before it can perform the
  // code_cvt conversions.
  // For "char" no allocations will be performed because no conversion is
  // required.
  bool DisableAllocations = std::is_same<CharT, char>::value;
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    InputIter RHS(R);
    {
      RequireAllocationGuard  g; // requires 1 or more allocations occur by default
      if (DisableAllocations) g.requireExactly(0);
      LHS += RHS;
    }
    assert(LHS == E);
  }
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    InputIter RHS(R);
    InputIter REnd(StrEnd(R));
    {
      RequireAllocationGuard g;
      if (DisableAllocations) g.requireExactly(0);
      LHS.concat(RHS, REnd);
    }
    assert(LHS == E);
  }
}

template <class CharT>
void doConcatSourceTest(ConcatOperatorTestcase const& TC)
{
  using namespace fs;
  using Ptr = CharT const*;
  using Str = std::basic_string<CharT>;
  using StrView = std::basic_string_view<CharT>;
  using InputIter = input_iterator<Ptr>;
  const Ptr L = TC.lhs;
  const Ptr R = TC.rhs;
  const Ptr E = TC.expect;
  // basic_string
  {
    path LHS(L);
    Str RHS(R);
    path& Ref = (LHS += RHS);
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
  {
    path LHS(L);
    Str RHS(R);
    path& Ref = LHS.concat(RHS);
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
  // basic_string_view
  {
    path LHS(L);
    StrView RHS(R);
    path& Ref = (LHS += RHS);
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
  {
    path LHS(L);
    StrView RHS(R);
    path& Ref = LHS.concat(RHS);
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
  // Char*
  {
    path LHS(L);
    Str RHS(R);
    path& Ref = (LHS += RHS);
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
  {
    path LHS(L);
    Ptr RHS(R);
    path& Ref = LHS.concat(RHS);
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
  {
    path LHS(L);
    Ptr RHS(R);
    path& Ref = LHS.concat(RHS, StrEnd(RHS));
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
  // iterators
  {
    path LHS(L);
    InputIter RHS(R);
    path& Ref = (LHS += RHS);
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
  {
    path LHS(L); InputIter RHS(R);
    path& Ref = LHS.concat(RHS);
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
  {
    path LHS(L);
    InputIter RHS(R);
    InputIter REnd(StrEnd(R));
    path& Ref = LHS.concat(RHS, REnd);
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
}

template <class CharT>
void doConcatECharTest(ConcatOperatorTestcase const& TC)
{
  using namespace fs;
  using Ptr = CharT const*;
  const Ptr RStr = TC.rhs;
  assert(StrLen(RStr) == 1);
  const Ptr L   = TC.lhs;
  const CharT R = RStr[0];
  const Ptr E   = TC.expect;
  {
    path LHS(L);
    path& Ref = (LHS += R);
    assert(LHS == E);
    assert(&Ref == &LHS);
  }
}


template <class It, class = decltype(fs::path{}.concat(std::declval<It>()))>
constexpr bool has_concat(int) { return true; }
template <class It>
constexpr bool has_concat(long) { return false; }

template <class It, class = decltype(fs::path{}.operator+=(std::declval<It>()))>
constexpr bool has_concat_op(int) { return true; }
template <class It>
constexpr bool has_concat_op(long) { return false; }
template <class It>
constexpr bool has_concat_op() { return has_concat_op<It>(0); }

template <class It>
constexpr bool has_concat() {
  static_assert(has_concat<It>(0) == has_concat_op<It>(0), "must be same");
  return has_concat<It>(0) && has_concat_op<It>(0);
}

void test_sfinae() {
  using namespace fs;
  {
    static_assert(has_concat_op<char>(), "");
    static_assert(has_concat_op<const char>(), "");
    static_assert(has_concat_op<char16_t>(), "");
    static_assert(has_concat_op<const char16_t>(), "");
  }
  {
    using It = const char* const;
    static_assert(has_concat<It>(), "");
  }
  {
    using It = input_iterator<const char*>;
    static_assert(has_concat<It>(), "");
  }
  {
    struct Traits {
      using iterator_category = std::input_iterator_tag;
      using value_type = const char;
      using pointer = const char*;
      using reference = const char&;
      using difference_type = std::ptrdiff_t;
    };
    using It = input_iterator<const char*, Traits>;
    static_assert(has_concat<It>(), "");
  }
  {
    using It = output_iterator<const char*>;
    static_assert(!has_concat<It>(), "");
  }
  {
    static_assert(!has_concat<int>(0), "");
    // operator+=(int) is well formed since it converts to operator+=(value_type)
    // but concat(int) isn't valid because there is no concat(value_type).
    // This should probably be addressed by a LWG issue.
    static_assert(has_concat_op<int>(), "");
  }
  {
    static_assert(!has_concat<int*>(), "");
  }
}

int main()
{
  using namespace fs;
  for (auto const & TC : Cases) {
    {
      path LHS((const char*)TC.lhs);
      path RHS((const char*)TC.rhs);
      path& Ref = (LHS += RHS);
      assert(LHS == (const char*)TC.expect);
      assert(&Ref == &LHS);
    }
    {
      path LHS((const char*)TC.lhs);
      std::string_view RHS((const char*)TC.rhs);
      path& Ref = (LHS += RHS);
      assert(LHS == (const char*)TC.expect);
      assert(&Ref == &LHS);
    }
    doConcatSourceTest<char>    (TC);
    doConcatSourceTest<wchar_t> (TC);
    doConcatSourceTest<char16_t>(TC);
    doConcatSourceTest<char32_t>(TC);
  }
  for (auto const & TC : LongLHSCases) {
    // Do path test
    {
      path LHS((const char*)TC.lhs);
      path RHS((const char*)TC.rhs);
      const char* E = TC.expect;
      PathReserve(LHS, StrLen(E) + 5);
      {
        DisableAllocationGuard g;
        path& Ref = (LHS += RHS);
        assert(&Ref == &LHS);
      }
      assert(LHS == E);
    }
    {
      path LHS((const char*)TC.lhs);
      std::string_view RHS((const char*)TC.rhs);
      const char* E = TC.expect;
      PathReserve(LHS, StrLen(E) + 5);
      {
        DisableAllocationGuard g;
        path& Ref = (LHS += RHS);
        assert(&Ref == &LHS);
      }
      assert(LHS == E);
    }
    doConcatSourceAllocTest<char>(TC);
    doConcatSourceAllocTest<wchar_t>(TC);
  }
  for (auto const& TC : CharTestCases) {
    doConcatECharTest<char>(TC);
    doConcatECharTest<wchar_t>(TC);
    doConcatECharTest<char16_t>(TC);
    doConcatECharTest<char32_t>(TC);
  }
  test_sfinae();
}
