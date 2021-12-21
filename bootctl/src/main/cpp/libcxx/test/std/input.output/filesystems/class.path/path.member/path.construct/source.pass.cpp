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

// template <class Source>
//      path(const Source& source);
// template <class InputIterator>
//      path(InputIterator first, InputIterator last);


#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "min_allocator.h"
#include "filesystem_test_helper.hpp"


template <class CharT, class ...Args>
void RunTestCaseImpl(MultiStringType const& MS, Args... args) {
  using namespace fs;
  const char* Expect = MS;
  const CharT* TestPath = MS;
  const CharT* TestPathEnd = StrEnd(TestPath);
  const std::size_t Size = TestPathEnd - TestPath;
  const std::size_t SSize = StrEnd(Expect) - Expect;
  assert(Size == SSize);
  // StringTypes
  {
    const std::basic_string<CharT> S(TestPath);
    path p(S, args...);
    assert(p.native() == Expect);
    assert(p.string<CharT>() == TestPath);
    assert(p.string<CharT>() == S);
  }
  {
    const std::basic_string_view<CharT> S(TestPath);
    path p(S, args...);
    assert(p.native() == Expect);
    assert(p.string<CharT>() == TestPath);
    assert(p.string<CharT>() == S);
  }
  // Char* pointers
  {
    path p(TestPath, args...);
    assert(p.native() == Expect);
    assert(p.string<CharT>() == TestPath);
  }
  {
    path p(TestPath, TestPathEnd, args...);
    assert(p.native() == Expect);
    assert(p.string<CharT>() == TestPath);
  }
  // Iterators
  {
    using It = input_iterator<const CharT*>;
    path p(It{TestPath}, args...);
    assert(p.native() == Expect);
    assert(p.string<CharT>() == TestPath);
  }
  {
    using It = input_iterator<const CharT*>;
    path p(It{TestPath}, It{TestPathEnd}, args...);
    assert(p.native() == Expect);
    assert(p.string<CharT>() == TestPath);
  }
}

template <class CharT, class ...Args>
void RunTestCase(MultiStringType const& MS) {
  RunTestCaseImpl<CharT>(MS);
  RunTestCaseImpl<CharT>(MS, fs::path::format::auto_format);
  RunTestCaseImpl<CharT>(MS, fs::path::format::native_format);
  RunTestCaseImpl<CharT>(MS, fs::path::format::generic_format);
}

void test_sfinae() {
  using namespace fs;
  {
    using It = const char* const;
    static_assert(std::is_constructible<path, It>::value, "");
  }
  {
    using It = input_iterator<const char*>;
    static_assert(std::is_constructible<path, It>::value, "");
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
    static_assert(std::is_constructible<path, It>::value, "");
  }
  {
    using It = output_iterator<const char*>;
    static_assert(!std::is_constructible<path, It>::value, "");

  }
  {
    static_assert(!std::is_constructible<path, int*>::value, "");
  }
}

int main() {
  for (auto const& MS : PathList) {
    RunTestCase<char>(MS);
    RunTestCase<wchar_t>(MS);
    RunTestCase<char16_t>(MS);
    RunTestCase<char32_t>(MS);
  }
  test_sfinae();
}
