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

#include <string>
#include <cassert>
#include <type_traits>

#include "test_macros.h"

template <class T>
void
test()
{
    typedef std::hash<T> H;
    static_assert((std::is_same<typename H::argument_type, T>::value), "" );
    static_assert((std::is_same<typename H::result_type, std::size_t>::value), "" );
    ASSERT_NOEXCEPT(H()(T()));

    H h;
    std::string g1 = "1234567890";
    std::string g2 = "1234567891";
    T s1(g1.begin(), g1.end());
    T s2(g2.begin(), g2.end());
    assert(h(s1) != h(s2));
}

int main()
{
    test<std::string>();
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    test<std::u8string>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<std::u16string>();
    test<std::u32string>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<std::wstring>();
}
