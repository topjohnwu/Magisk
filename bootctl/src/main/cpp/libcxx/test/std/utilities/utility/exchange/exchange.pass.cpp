//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <utility>

// exchange

// template<class T, class U=T>
//    constexpr T            // constexpr after C++17
//    exchange(T& obj, U&& new_value);

#include <utility>
#include <cassert>
#include <string>

#include "test_macros.h"

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int v = 12;

    if (12 != std::exchange(v,23) || v != 23)
        return false;

    if (23 != std::exchange(v,static_cast<short>(67)) || v != 67)
        return false;

    if (67 != std::exchange<int, short>(v, {}) || v != 0)
        return false;
    return true;
    }
#endif



int main()
{
    {
    int v = 12;
    assert ( std::exchange ( v, 23 ) == 12 );
    assert ( v == 23 );
    assert ( std::exchange ( v, static_cast<short>(67) ) == 23 );
    assert ( v == 67 );

    assert ((std::exchange<int, short> ( v, {} )) == 67 );
    assert ( v == 0 );

    }

    {
    bool b = false;
    assert ( !std::exchange ( b, true ));
    assert ( b );
    }

    {
    const std::string s1 ( "Hi Mom!" );
    const std::string s2 ( "Yo Dad!" );
    std::string s3 = s1; // Mom
    assert ( std::exchange ( s3, s2 ) == s1 );
    assert ( s3 == s2 );
    assert ( std::exchange ( s3, "Hi Mom!" ) == s2 );
    assert ( s3 == s1 );

    s3 = s2; // Dad
    assert ( std::exchange ( s3, {} ) == s2 );
    assert ( s3.size () == 0 );

    s3 = s2; // Dad
    assert ( std::exchange ( s3, "" ) == s2 );
    assert ( s3.size () == 0 );
    }

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
