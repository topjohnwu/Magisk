//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// template <class T>
// struct hash
//     : public unary_function<T, size_t>
// {
//     size_t operator()(T val) const;
// };

// Not very portable

#include <string_view>
#include <string>
#include <cassert>
#include <type_traits>

#include "test_macros.h"

using std::string_view;

template <class SV>
void
test()
{
    typedef std::hash<SV> H;
    static_assert((std::is_same<typename H::argument_type, SV>::value), "" );
    static_assert((std::is_same<typename H::result_type, std::size_t>::value), "" );

    typedef typename SV::value_type char_type;
    typedef std::basic_string<char_type> String;
    typedef std::hash<String> SH;
    ASSERT_NOEXCEPT(H()(SV()));

    char_type g1 [ 10 ];
    char_type g2 [ 10 ];
    for ( int i = 0; i < 10; ++i )
        g1[i] = g2[9-i] = static_cast<char_type>('0' + i);
    H h;
    SH sh;
    SV s1(g1, 10);
    String ss1(s1);
    SV s2(g2, 10);
    String ss2(s2);
    assert(h(s1) == h(s1));
    assert(h(s1) != h(s2));
    assert(sh(ss1) == h(s1));
    assert(sh(ss2) == h(s2));
}

int main()
{
    test<std::string_view>();
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    test<std::u8string_view>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<std::u16string_view>();
    test<std::u32string_view>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<std::wstring_view>();
}
