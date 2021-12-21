//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// constexpr const_iterator begin() const;

#include <string_view>
#include <cassert>

#include "test_macros.h"

template <class S>
void
test(S s)
{
    const S& cs = s;
    typename S::iterator b = s.begin();
    typename S::const_iterator cb1 = cs.begin();
    typename S::const_iterator cb2 = s.cbegin();
    if (!s.empty())
    {
        assert(   *b ==  s[0]);
        assert(  &*b == &s[0]);
        assert( *cb1 ==  s[0]);
        assert(&*cb1 == &s[0]);
        assert( *cb2 ==  s[0]);
        assert(&*cb2 == &s[0]);

    }
    assert(  b == cb1);
    assert(  b == cb2);
    assert(cb1 == cb2);
}


int main()
{
    typedef std::string_view    string_view;
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    typedef std::u8string_view  u8string_view;
#endif
    typedef std::u16string_view u16string_view;
    typedef std::u32string_view u32string_view;
    typedef std::wstring_view   wstring_view;

    test(string_view   ());
    test(u16string_view());
    test(u32string_view());
    test(wstring_view  ());
    test(string_view   ( "123"));
    test(wstring_view  (L"123"));
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    test(u8string_view{u8"123"});
#endif
#if TEST_STD_VER >= 11
    test(u16string_view{u"123"});
    test(u32string_view{U"123"});
#endif

#if TEST_STD_VER > 11
    {
    constexpr string_view       sv { "123", 3 };
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    constexpr u8string_view u8sv  {u8"123", 3 };
#endif
    constexpr u16string_view u16sv {u"123", 3 };
    constexpr u32string_view u32sv {U"123", 3 };
    constexpr wstring_view     wsv {L"123", 3 };

    static_assert (    *sv.begin() ==    sv[0], "" );
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    static_assert (  *u8sv.begin() ==  u8sv[0], "" );
#endif
    static_assert ( *u16sv.begin() == u16sv[0], "" );
    static_assert ( *u32sv.begin() == u32sv[0], "" );
    static_assert (   *wsv.begin() ==   wsv[0], "" );

    static_assert (    *sv.cbegin() ==    sv[0], "" );
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    static_assert (  *u8sv.cbegin() ==  u8sv[0], "" );
#endif
    static_assert ( *u16sv.cbegin() == u16sv[0], "" );
    static_assert ( *u32sv.cbegin() == u32sv[0], "" );
    static_assert (   *wsv.cbegin() ==   wsv[0], "" );
    }
#endif
}
