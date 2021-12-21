//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-deduction-guides

// <string>

// Test that the constructors offered by std::basic_string are formulated
// so they're compatible with implicit deduction guides.

#include <string>
#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"
#include "test_iterators.h"
#include "constexpr_char_traits.hpp"

template <class T, class Alloc = std::allocator<T>>
using BStr = std::basic_string<T, std::char_traits<T>, Alloc>;

// Overloads
//  using A = Allocator;
//  using BS = basic_string
//  using BSV = basic_string_view
// ---------------
// (1)  basic_string() - NOT TESTED
// (2)  basic_string(A const&) - BROKEN
// (3)  basic_string(size_type, CharT, const A& = A())
// (4)  basic_string(BS const&, size_type, A const& = A())
// (5)  basic_string(BS const&, size_type, size_type, A const& = A())
// (6)  basic_string(const CharT*, size_type, A const& = A())
// (7)  basic_string(const CharT*, A const& = A())
// (8)  basic_string(InputIt, InputIt, A const& = A()) - BROKEN
// (9)  basic_string(BS const&)
// (10) basic_string(BS const&, A const&)
// (11) basic_string(BS&&)
// (12) basic_string(BS&&, A const&)
// (13) basic_string(initializer_list<CharT>, A const& = A())
// (14) basic_string(BSV, A const& = A())
// (15) basic_string(const T&, size_type, size_type, A const& = A())
int main()
{
  using TestSizeT = test_allocator<char>::size_type;
  { // Testing (1)
    // Nothing TODO. Cannot deduce without any arguments.
  }
  { // Testing (2)
    // This overload isn't compatible with implicit deduction guides as
    // specified in the standard.
    // const test_allocator<char> alloc{};
    // std::basic_string s(alloc);
  }
  { // Testing (3) w/o allocator
    std::basic_string s(6ull, 'a');
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "aaaaaa");

    std::basic_string w(2ull, L'b');
    ASSERT_SAME_TYPE(decltype(w), std::wstring);
    assert(w == L"bb");
  }
  { // Testing (3) w/ allocator
    std::basic_string s(6ull, 'a', test_allocator<char>{});
    ASSERT_SAME_TYPE(decltype(s), BStr<char,test_allocator<char>>);
    assert(s == "aaaaaa");

    std::basic_string w(2ull, L'b', test_allocator<wchar_t>{});
    ASSERT_SAME_TYPE(decltype(w), BStr<wchar_t, test_allocator<wchar_t>>);
    assert(w == L"bb");
  }
  { // Testing (4) w/o allocator
    const std::string sin("abc");
    std::basic_string s(sin, (size_t)1);
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "bc");

    using WStr = std::basic_string<wchar_t,
                                  constexpr_char_traits<wchar_t>,
                                  test_allocator<wchar_t>>;
    const WStr win(L"abcdef");
    std::basic_string w(win, (TestSizeT)3);
    ASSERT_SAME_TYPE(decltype(w), WStr);
    assert(w == L"def");
  }
  { // Testing (4) w/ allocator
    const std::string sin("abc");
    std::basic_string s(sin, (size_t)1, std::allocator<char>{});
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "bc");

    using WStr = std::basic_string<wchar_t,
                                  constexpr_char_traits<wchar_t>,
                                  test_allocator<wchar_t>>;
    const WStr win(L"abcdef");
    std::basic_string w(win, (TestSizeT)3, test_allocator<wchar_t>{});
    ASSERT_SAME_TYPE(decltype(w), WStr);
    assert(w == L"def");
  }
  { // Testing (5) w/o allocator
    const std::string sin("abc");
    std::basic_string s(sin, (size_t)1, (size_t)3);
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "bc");

    using WStr = std::basic_string<wchar_t,
                                  constexpr_char_traits<wchar_t>,
                                  test_allocator<wchar_t>>;
    const WStr win(L"abcdef");
    std::basic_string w(win, (TestSizeT)2, (TestSizeT)3);
    ASSERT_SAME_TYPE(decltype(w), WStr);
    assert(w == L"cde");
  }
  { // Testing (5) w/ allocator
    const std::string sin("abc");
    std::basic_string s(sin, (size_t)1, (size_t)3, std::allocator<char>{});
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "bc");

    using WStr = std::basic_string<wchar_t,
                                  constexpr_char_traits<wchar_t>,
                                  test_allocator<wchar_t>>;
    const WStr win(L"abcdef");
    std::basic_string w(win, (TestSizeT)2, (TestSizeT)3, test_allocator<wchar_t>{});
    ASSERT_SAME_TYPE(decltype(w), WStr);
    assert(w == L"cde");
  }
  { // Testing (6) w/o allocator
    std::basic_string s("abc", (size_t)2);
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "ab");

    std::basic_string w(L"abcdef", (size_t)3);
    ASSERT_SAME_TYPE(decltype(w), std::wstring);
    assert(w == L"abc");
  }
  { // Testing (6) w/ allocator
    std::basic_string s("abc", (size_t)2, std::allocator<char>{});
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "ab");

    using WStr = std::basic_string<wchar_t,
                                  std::char_traits<wchar_t>,
                                  test_allocator<wchar_t>>;
    std::basic_string w(L"abcdef", (TestSizeT)3, test_allocator<wchar_t>{});
    ASSERT_SAME_TYPE(decltype(w), WStr);
    assert(w == L"abc");
  }
  { // Testing (7) w/o allocator
    std::basic_string s("abc");
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "abc");

    std::basic_string w(L"abcdef");
    ASSERT_SAME_TYPE(decltype(w), std::wstring);
    assert(w == L"abcdef");
  }
  { // Testing (7) w/ allocator
    std::basic_string s("abc", std::allocator<char>{});
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "abc");

    using WStr = std::basic_string<wchar_t,
                                  std::char_traits<wchar_t>,
                                  test_allocator<wchar_t>>;
    std::basic_string w(L"abcdef", test_allocator<wchar_t>{});
    ASSERT_SAME_TYPE(decltype(w), WStr);
    assert(w == L"abcdef");
  }
  { // (8) w/o allocator
    using It = input_iterator<const char*>;
    const char* input = "abcdef";
    std::basic_string s(It(input), It(input + 3), std::allocator<char>{});
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "abc");
  }
  { // (8) w/ allocator
    using ExpectW = std::basic_string<wchar_t, std::char_traits<wchar_t>, test_allocator<wchar_t>>;
    using It = input_iterator<const wchar_t*>;
    const wchar_t* input = L"abcdef";
    std::basic_string s(It(input), It(input + 3), test_allocator<wchar_t>{});
    ASSERT_SAME_TYPE(decltype(s), ExpectW);
    assert(s == L"abc");
  }
  { // Testing (9)
    const std::string sin("abc");
    std::basic_string s(sin);
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "abc");

    using WStr = std::basic_string<wchar_t,
                                  constexpr_char_traits<wchar_t>,
                                  test_allocator<wchar_t>>;
    const WStr win(L"abcdef");
    std::basic_string w(win);
    ASSERT_SAME_TYPE(decltype(w), WStr);
    assert(w == L"abcdef");
  }
  { // Testing (10)
    const std::string sin("abc");
    std::basic_string s(sin, std::allocator<char>{});
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "abc");

    using WStr = std::basic_string<wchar_t,
                                  constexpr_char_traits<wchar_t>,
                                  test_allocator<wchar_t>>;
    const WStr win(L"abcdef");
    std::basic_string w(win, test_allocator<wchar_t>{});
    ASSERT_SAME_TYPE(decltype(w), WStr);
    assert(w == L"abcdef");
  }
  { // Testing (11)
    std::string sin("abc");
    std::basic_string s(std::move(sin));
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "abc");

    using WStr = std::basic_string<wchar_t,
                                  constexpr_char_traits<wchar_t>,
                                  test_allocator<wchar_t>>;
    WStr win(L"abcdef");
    std::basic_string w(std::move(win));
    ASSERT_SAME_TYPE(decltype(w), WStr);
    assert(w == L"abcdef");
  }
  { // Testing (12)
    std::string sin("abc");
    std::basic_string s(std::move(sin), std::allocator<char>{});
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "abc");

    using WStr = std::basic_string<wchar_t,
                                  constexpr_char_traits<wchar_t>,
                                  test_allocator<wchar_t>>;
    WStr win(L"abcdef");
    std::basic_string w(std::move(win), test_allocator<wchar_t>{});
    ASSERT_SAME_TYPE(decltype(w), WStr);
    assert(w == L"abcdef");
  }
  { // Testing (13) w/o allocator
    std::basic_string s({'a', 'b', 'c'});
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "abc");

    std::basic_string w({L'a', L'b', L'c'});
    ASSERT_SAME_TYPE(decltype(w), std::wstring);
    assert(w == L"abc");
  }
  { // Testing (13) w/ allocator
    std::basic_string s({'a', 'b', 'c'}, test_allocator<char>{});
    ASSERT_SAME_TYPE(decltype(s), BStr<char, test_allocator<char>>);
    assert(s == "abc");

    std::basic_string w({L'a', L'b', L'c'}, test_allocator<wchar_t>{});
    ASSERT_SAME_TYPE(decltype(w), BStr<wchar_t, test_allocator<wchar_t>>);
    assert(w == L"abc");
  }
  { // Testing (14) w/o allocator
    std::string_view sv("abc");
    std::basic_string s(sv);
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "abc");

    using Expect = std::basic_string<wchar_t, constexpr_char_traits<wchar_t>>;
    std::basic_string_view<wchar_t, constexpr_char_traits<wchar_t>> BSV(L"abcdef");
    std::basic_string w(BSV);
    ASSERT_SAME_TYPE(decltype(w), Expect);
    assert(w == L"abcdef");
  }
  { // Testing (14) w/ allocator
    using ExpectS = std::basic_string<char, std::char_traits<char>, test_allocator<char>>;
    std::string_view sv("abc");
    std::basic_string s(sv, test_allocator<char>{});
    ASSERT_SAME_TYPE(decltype(s), ExpectS);
    assert(s == "abc");

    using ExpectW = std::basic_string<wchar_t, constexpr_char_traits<wchar_t>,
                                      test_allocator<wchar_t>>;
    std::basic_string_view<wchar_t, constexpr_char_traits<wchar_t>> BSV(L"abcdef");
    std::basic_string w(BSV, test_allocator<wchar_t>{});
    ASSERT_SAME_TYPE(decltype(w), ExpectW);
    assert(w == L"abcdef");
  }
  { // Testing (15) w/o allocator
    std::string s0("abc");
    std::basic_string s(s0, 1, 1);
    ASSERT_SAME_TYPE(decltype(s), std::string);
    assert(s == "b");

    std::wstring w0(L"abcdef");
    std::basic_string w(w0, 2, 2);
    ASSERT_SAME_TYPE(decltype(w), std::wstring);
    assert(w == L"cd");
  }
  { // Testing (15) w/ allocator
    using ExpectS = std::basic_string<char, std::char_traits<char>, test_allocator<char>>;
    ExpectS s0("abc");
    std::basic_string s(s0, 1, 1, test_allocator<char>{4});
    ASSERT_SAME_TYPE(decltype(s), ExpectS);
    assert(s == "b");

    using ExpectW = std::basic_string<wchar_t, std::char_traits<wchar_t>, test_allocator<wchar_t>>;
    ExpectW w0(L"abcdef");
    std::basic_string w(w0, 2, 2, test_allocator<wchar_t>{6});
    ASSERT_SAME_TYPE(decltype(w), ExpectW);
    assert(w == L"cd");
  }
}
