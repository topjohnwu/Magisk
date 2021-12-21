// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// UNSUPPORTED: clang-3.3, clang-3.4, clang-3.5, clang-3.6, clang-3.7, clang-3.8, clang-3.9
// UNSUPPORTED: apple-clang-6, apple-clang-7, apple-clang-8
// Note: libc++ supports string_view before C++17, but literals were introduced in C++14

#include <string_view>
#include <cassert>

#include "test_macros.h"

#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    typedef std::u8string_view u8string_view;
#else
    typedef std::string_view   u8string_view;
#endif

int main()
{
    using namespace std::literals::string_view_literals;

    static_assert ( std::is_same<decltype(   "Hi"sv), std::string_view>::value, "" );
    static_assert ( std::is_same<decltype( u8"Hi"sv), u8string_view>::value, "" );
    static_assert ( std::is_same<decltype(  L"Hi"sv), std::wstring_view>::value, "" );
    static_assert ( std::is_same<decltype(  u"Hi"sv), std::u16string_view>::value, "" );
    static_assert ( std::is_same<decltype(  U"Hi"sv), std::u32string_view>::value, "" );

    std::string_view foo;
    std::wstring_view Lfoo;
    u8string_view u8foo;
    std::u16string_view ufoo;
    std::u32string_view Ufoo;

    
    foo  =    ""sv;     assert(  foo.size() == 0);
    u8foo = u8""sv;     assert(u8foo.size() == 0);
    Lfoo  =  L""sv;     assert( Lfoo.size() == 0);
    ufoo  =  u""sv;     assert( ufoo.size() == 0);
    Ufoo  =  U""sv;     assert( Ufoo.size() == 0);

    foo   =   " "sv;    assert(  foo.size() == 1);
    u8foo = u8" "sv;    assert(u8foo.size() == 1);
    Lfoo  =  L" "sv;    assert( Lfoo.size() == 1);
    ufoo  =  u" "sv;    assert( ufoo.size() == 1);
    Ufoo  =  U" "sv;    assert( Ufoo.size() == 1);

    foo   =   "ABC"sv;  assert(  foo ==   "ABC");   assert(  foo == std::string_view   (  "ABC"));
    u8foo = u8"ABC"sv;  assert(u8foo == u8"ABC");   assert(u8foo == u8string_view      (u8"ABC"));
    Lfoo  =  L"ABC"sv;  assert( Lfoo ==  L"ABC");   assert( Lfoo == std::wstring_view  ( L"ABC"));
    ufoo  =  u"ABC"sv;  assert( ufoo ==  u"ABC");   assert( ufoo == std::u16string_view( u"ABC"));
    Ufoo  =  U"ABC"sv;  assert( Ufoo ==  U"ABC");   assert( Ufoo == std::u32string_view( U"ABC"));

    static_assert(  "ABC"sv.size() == 3, "");
    static_assert(u8"ABC"sv.size() == 3, "");
    static_assert( L"ABC"sv.size() == 3, "");
    static_assert( u"ABC"sv.size() == 3, "");
    static_assert( U"ABC"sv.size() == 3, "");

    static_assert(noexcept(  "ABC"sv), "");
    static_assert(noexcept(u8"ABC"sv), "");
    static_assert(noexcept( L"ABC"sv), "");
    static_assert(noexcept( u"ABC"sv), "");
    static_assert(noexcept( U"ABC"sv), "");
}
