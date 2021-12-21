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

// path& operator/=(path const&)
// template <class Source>
//      path& operator/=(Source const&);
// template <class Source>
//      path& append(Source const&);
// template <class InputIterator>
//      path& append(InputIterator first, InputIterator last);


#include "filesystem_include.hpp"
#include <type_traits>
#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "filesystem_test_helper.hpp"
#include "verbose_assert.h"


struct AppendOperatorTestcase {
  MultiStringType lhs;
  MultiStringType rhs;
  MultiStringType expect;
};

#define S(Str) MKSTR(Str)
const AppendOperatorTestcase Cases[] =
    {
        {S(""),     S(""),      S("")}
      , {S("p1"),   S("p2"),    S("p1/p2")}
      , {S("p1/"),  S("p2"),    S("p1/p2")}
      , {S("p1"),   S("/p2"),   S("/p2")}
      , {S("p1/"),  S("/p2"),   S("/p2")}
      , {S("p1"),   S("\\p2"),  S("p1/\\p2")}
      , {S("p1\\"), S("p2"),  S("p1\\/p2")}
      , {S("p1\\"), S("\\p2"),  S("p1\\/\\p2")}
      , {S(""),     S("p2"),    S("p2")}
      , {S("/p1"),  S("p2"),    S("/p1/p2")}
      , {S("/p1"),  S("/p2"),    S("/p2")}
      , {S("/p1/p3"),  S("p2"),    S("/p1/p3/p2")}
      , {S("/p1/p3/"),  S("p2"),    S("/p1/p3/p2")}
      , {S("/p1/"),  S("p2"),    S("/p1/p2")}
      , {S("/p1/p3/"),  S("/p2/p4"),    S("/p2/p4")}
      , {S("/"),    S(""),      S("/")}
      , {S("/p1"), S("/p2/"), S("/p2/")}
      , {S("p1"),   S(""),      S("p1/")}
      , {S("p1/"),  S(""),      S("p1/")}
    };


const AppendOperatorTestcase LongLHSCases[] =
    {
        {S("p1"),   S("p2"),    S("p1/p2")}
      , {S("p1/"),  S("p2"),    S("p1/p2")}
      , {S("p1"),   S("/p2"),   S("/p2")}
      , {S("/p1"),  S("p2"),    S("/p1/p2")}
    };
#undef S


// The append operator may need to allocate a temporary buffer before a code_cvt
// conversion. Test if this allocation occurs by:
//   1. Create a path, `LHS`, and reserve enough space to append `RHS`.
//      This prevents `LHS` from allocating during the actual appending.
//   2. Create a `Source` object `RHS`, which represents a "large" string.
//      (The string must not trigger the SSO)
//   3. Append `RHS` to `LHS` and check for the expected allocation behavior.
template <class CharT>
void doAppendSourceAllocTest(AppendOperatorTestcase const& TC)
{
  using namespace fs;
  using Ptr = CharT const*;
  using Str = std::basic_string<CharT>;
  using StrView = std::basic_string_view<CharT>;
  using InputIter = input_iterator<Ptr>;

  const Ptr L = TC.lhs;
  Str RShort = (Ptr)TC.rhs;
  Str EShort = (Ptr)TC.expect;
  assert(RShort.size() >= 2);
  CharT c = RShort.back();
  RShort.append(100, c);
  EShort.append(100, c);
  const Ptr R = RShort.data();
  const Str& E = EShort;
  std::size_t ReserveSize = E.size() + 3;
  // basic_string
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    Str  RHS(R);
    {
      DisableAllocationGuard g;
      LHS /= RHS;
    }
    ASSERT_PRED(PathEq, LHS , E);
  }
  // basic_string_view
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    StrView  RHS(R);
    {
      DisableAllocationGuard g;
      LHS /= RHS;
    }
    assert(PathEq(LHS, E));
  }
  // CharT*
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    Ptr RHS(R);
    {
      DisableAllocationGuard g;
      LHS /= RHS;
    }
    assert(PathEq(LHS, E));
  }
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    Ptr RHS(R);
    {
      DisableAllocationGuard g;
      LHS.append(RHS, StrEnd(RHS));
    }
    assert(PathEq(LHS, E));
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
      LHS /= RHS;
    }
    assert(PathEq(LHS, E));
  }
  {
    path LHS(L); PathReserve(LHS, ReserveSize);
    InputIter RHS(R);
    InputIter REnd(StrEnd(R));
    {
      RequireAllocationGuard g;
      if (DisableAllocations) g.requireExactly(0);
      LHS.append(RHS, REnd);
    }
    assert(PathEq(LHS, E));
  }
}

template <class CharT>
void doAppendSourceTest(AppendOperatorTestcase const& TC)
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
    path Result(L);
    Str RHS(R);
    path& Ref = (Result /= RHS);
    ASSERT_EQ(Result, E)
        << DISPLAY(L) << DISPLAY(R);
    assert(&Ref == &Result);
  }
  {
    path LHS(L);
    Str RHS(R);
    path& Ref = LHS.append(RHS);
    assert(PathEq(LHS, E));
    assert(&Ref == &LHS);
  }
  // basic_string_view
  {
    path LHS(L);
    StrView RHS(R);
    path& Ref = (LHS /= RHS);
    assert(PathEq(LHS, E));
    assert(&Ref == &LHS);
  }
  {
    path LHS(L);
    StrView RHS(R);
    path& Ref = LHS.append(RHS);
    assert(PathEq(LHS, E));
    assert(&Ref == &LHS);
  }
  // Char*
  {
    path LHS(L);
    Str RHS(R);
    path& Ref = (LHS /= RHS);
    assert(PathEq(LHS, E));
    assert(&Ref == &LHS);
  }
  {
    path LHS(L);
    Ptr RHS(R);
    path& Ref = LHS.append(RHS);
    assert(PathEq(LHS, E));
    assert(&Ref == &LHS);
  }
  {
    path LHS(L);
    Ptr RHS(R);
    path& Ref = LHS.append(RHS, StrEnd(RHS));
    ASSERT_PRED(PathEq, LHS, E)
        << DISPLAY(L) << DISPLAY(R);
    assert(&Ref == &LHS);
  }
  // iterators
  {
    path LHS(L);
    InputIter RHS(R);
    path& Ref = (LHS /= RHS);
    assert(PathEq(LHS, E));
    assert(&Ref == &LHS);
  }
  {
    path LHS(L); InputIter RHS(R);
    path& Ref = LHS.append(RHS);
    assert(PathEq(LHS, E));
    assert(&Ref == &LHS);
  }
  {
    path LHS(L);
    InputIter RHS(R);
    InputIter REnd(StrEnd(R));
    path& Ref = LHS.append(RHS, REnd);
    assert(PathEq(LHS, E));
    assert(&Ref == &LHS);
  }
}



template <class It, class = decltype(fs::path{}.append(std::declval<It>()))>
constexpr bool has_append(int) { return true; }
template <class It>
constexpr bool has_append(long) { return false; }

template <class It, class = decltype(fs::path{}.operator/=(std::declval<It>()))>
constexpr bool has_append_op(int) { return true; }
template <class It>
constexpr bool has_append_op(long) { return false; }

template <class It>
constexpr bool has_append() {
  static_assert(has_append<It>(0) == has_append_op<It>(0), "must be same");
  return has_append<It>(0) && has_append_op<It>(0);
}

void test_sfinae()
{
  using namespace fs;
  {
    using It = const char* const;
    static_assert(has_append<It>(), "");
  }
  {
    using It = input_iterator<const char*>;
    static_assert(has_append<It>(), "");
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
    static_assert(has_append<It>(), "");
  }
  {
    using It = output_iterator<const char*>;
    static_assert(!has_append<It>(), "");

  }
  {
    static_assert(!has_append<int*>(), "");
  }
  {
    static_assert(!has_append<char>(), "");
    static_assert(!has_append<const char>(), "");
  }
}

int main()
{
  using namespace fs;
  for (auto const & TC : Cases) {
    {
      const char* LHS_In = TC.lhs;
      const char* RHS_In = TC.rhs;
      path LHS(LHS_In);
      path RHS(RHS_In);
      path& Res = (LHS /= RHS);
      ASSERT_PRED(PathEq, Res, (const char*)TC.expect)
          << DISPLAY(LHS_In) << DISPLAY(RHS_In);
      assert(&Res == &LHS);
    }
    doAppendSourceTest<char>    (TC);
    doAppendSourceTest<wchar_t> (TC);
    doAppendSourceTest<char16_t>(TC);
    doAppendSourceTest<char32_t>(TC);
  }
  for (auto const & TC : LongLHSCases) {
    doAppendSourceAllocTest<char>(TC);
    doAppendSourceAllocTest<wchar_t>(TC);
  }
  test_sfinae();
}
