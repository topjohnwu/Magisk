//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


// <string_view>

// constexpr basic_string_view () noexcept;

#include <string_view>
#include <cassert>

#include "test_macros.h"

template<typename T>
void test () {
#if TEST_STD_VER > 11
    {
    ASSERT_NOEXCEPT(T());

    constexpr T sv1;
    static_assert ( sv1.size() == 0, "" );
    static_assert ( sv1.empty(), "");
    }
#endif

    {
    T sv1;
    assert ( sv1.size() == 0 );
    assert ( sv1.empty());
    }
}

int main () {
    test<std::string_view> ();
    test<std::u16string_view> ();
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    test<std::u8string_view> ();
#endif
    test<std::u32string_view> ();
    test<std::wstring_view> ();

}
