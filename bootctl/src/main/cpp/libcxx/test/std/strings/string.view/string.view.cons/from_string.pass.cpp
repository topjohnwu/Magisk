//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


// <string_view>

// template<class Allocator>
// basic_string_view(const basic_string<_CharT, _Traits, Allocator>& _str) noexcept


#include <string_view>
#include <string>
#include <cassert>

#include "test_macros.h"

struct dummy_char_traits : public std::char_traits<char> {};

template<typename CharT, typename Traits>
void test ( const std::basic_string<CharT, Traits> &str ) {
    typedef std::basic_string_view<CharT, Traits> SV;
    ASSERT_NOEXCEPT(SV(str));

    SV sv1 ( str );
    assert ( sv1.size() == str.size());
    assert ( sv1.data() == str.data());
}

int main () {

    test ( std::string("QBCDE") );
    test ( std::string("") );
    test ( std::string() );

    test ( std::wstring(L"QBCDE") );
    test ( std::wstring(L"") );
    test ( std::wstring() );

#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    test ( std::u8string{u8"QBCDE"} );
    test ( std::u8string{u8""} );
    test ( std::u8string{} );
#endif

#if TEST_STD_VER >= 11
    test ( std::u16string{u"QBCDE"} );
    test ( std::u16string{u""} );
    test ( std::u16string{} );

    test ( std::u32string{U"QBCDE"} );
    test ( std::u32string{U""} );
    test ( std::u32string{} );
#endif

    test ( std::basic_string<char, dummy_char_traits>("QBCDE") );
    test ( std::basic_string<char, dummy_char_traits>("") );
    test ( std::basic_string<char, dummy_char_traits>() );

}
