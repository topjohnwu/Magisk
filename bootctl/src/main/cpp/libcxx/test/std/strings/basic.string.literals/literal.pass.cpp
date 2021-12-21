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

#include <string>
#include <cassert>

#include "test_macros.h"

#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    typedef std::u8string u8string;
#else
    typedef std::string   u8string;
#endif


int main()
{
    using namespace std::literals::string_literals;

    static_assert ( std::is_same<decltype(   "Hi"s), std::string>::value, "" );
    static_assert ( std::is_same<decltype( u8"Hi"s), u8string>::value, "" );
    static_assert ( std::is_same<decltype(  L"Hi"s), std::wstring>::value, "" );
    static_assert ( std::is_same<decltype(  u"Hi"s), std::u16string>::value, "" );
    static_assert ( std::is_same<decltype(  U"Hi"s), std::u32string>::value, "" );

    std::string foo;
    std::wstring Lfoo;
    u8string u8foo;
    std::u16string ufoo;
    std::u32string Ufoo;

    foo   =   ""s;     assert(  foo.size() == 0);
    u8foo = u8""s;     assert(u8foo.size() == 0);
    Lfoo  =  L""s;     assert( Lfoo.size() == 0);
    ufoo  =  u""s;     assert( ufoo.size() == 0);
    Ufoo  =  U""s;     assert( Ufoo.size() == 0);

    foo   =   " "s;    assert(  foo.size() == 1);
    u8foo = u8" "s;    assert(u8foo.size() == 1);
    Lfoo  =  L" "s;    assert( Lfoo.size() == 1);
    ufoo  =  u" "s;    assert( ufoo.size() == 1);
    Ufoo  =  U" "s;    assert( Ufoo.size() == 1);

    foo   =   "ABC"s;     assert(  foo ==   "ABC");   assert(  foo == std::string   (  "ABC"));
    u8foo = u8"ABC"s;     assert(u8foo == u8"ABC");   assert(u8foo == u8string      (u8"ABC"));
    Lfoo  =  L"ABC"s;     assert( Lfoo ==  L"ABC");   assert( Lfoo == std::wstring  ( L"ABC"));
    ufoo  =  u"ABC"s;     assert( ufoo ==  u"ABC");   assert( ufoo == std::u16string( u"ABC"));
    Ufoo  =  U"ABC"s;     assert( Ufoo ==  U"ABC");   assert( Ufoo == std::u32string( U"ABC"));
}
