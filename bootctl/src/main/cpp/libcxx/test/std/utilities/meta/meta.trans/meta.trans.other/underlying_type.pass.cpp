//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// underlying_type

#include <type_traits>
#include <climits>

#include "test_macros.h"

enum E { V = INT_MIN };

#if !defined(_WIN32) || defined(__MINGW32__)
    #define TEST_UNSIGNED_UNDERLYING_TYPE 1
#else
    #define TEST_UNSIGNED_UNDERLYING_TYPE 0 // MSVC's ABI doesn't follow the Standard
#endif

#if TEST_UNSIGNED_UNDERLYING_TYPE
enum F { W = UINT_MAX };
#endif // TEST_UNSIGNED_UNDERLYING_TYPE

int main()
{
    static_assert((std::is_same<std::underlying_type<E>::type, int>::value),
                  "E has the wrong underlying type");
#if TEST_UNSIGNED_UNDERLYING_TYPE
    static_assert((std::is_same<std::underlying_type<F>::type, unsigned>::value),
                  "F has the wrong underlying type");
#endif // TEST_UNSIGNED_UNDERLYING_TYPE

#if TEST_STD_VER > 11
    static_assert((std::is_same<std::underlying_type_t<E>, int>::value), "");
#if TEST_UNSIGNED_UNDERLYING_TYPE
    static_assert((std::is_same<std::underlying_type_t<F>, unsigned>::value), "");
#endif // TEST_UNSIGNED_UNDERLYING_TYPE
#endif // TEST_STD_VER > 11

#if TEST_STD_VER >= 11
    enum G : char { };

    static_assert((std::is_same<std::underlying_type<G>::type, char>::value),
                  "G has the wrong underlying type");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::underlying_type_t<G>, char>::value), "");
#endif // TEST_STD_VER > 11
#endif // TEST_STD_VER >= 11
}
