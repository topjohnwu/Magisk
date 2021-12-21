//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


// <string_view>

// constexpr basic_string_view& operator=(const basic_string_view &) noexcept = default;

#include <string_view>
#include <cassert>

#include "test_macros.h"

template<typename T>
#if TEST_STD_VER > 11
constexpr
#endif
bool test (T sv0)
    {
    T sv1;
    sv1 = sv0;
//  We can't just say "sv0 == sv1" here because string_view::compare
//  isn't constexpr until C++17, and we want to support back to C++14
    return sv0.size() == sv1.size() && sv0.data() == sv1.data();
    }

int main () {

    assert( test<std::string_view>    (  "1234"));
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    assert( test<std::u8string_view>  (u8"1234"));
#endif
#if TEST_STD_VER >= 11
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    assert( test<std::u16string_view> ( u"1234"));
    assert( test<std::u32string_view> ( U"1234"));
#endif
#endif
    assert( test<std::wstring_view>   ( L"1234"));

#if TEST_STD_VER > 11
    static_assert( test<std::string_view>    ({  "abc", 3}), "");
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    static_assert( test<std::u8string_view>  ({u8"abc", 3}), "");
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    static_assert( test<std::u16string_view> ({ u"abc", 3}), "");
    static_assert( test<std::u32string_view> ({ U"abc", 3}), "");
#endif
    static_assert( test<std::wstring_view>   ({ L"abc", 3}), "");
#endif
}
