//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// remove_extent

#include <type_traits>

#include "test_macros.h"

enum Enum {zero, one_};

template <class T, class U>
void test_remove_extent()
{
    static_assert((std::is_same<typename std::remove_extent<T>::type, U>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::remove_extent_t<T>,     U>::value), "");
#endif
}


int main()
{
    test_remove_extent<int, int> ();
    test_remove_extent<const Enum, const Enum> ();
    test_remove_extent<int[], int> ();
    test_remove_extent<const int[], const int> ();
    test_remove_extent<int[3], int> ();
    test_remove_extent<const int[3], const int> ();
    test_remove_extent<int[][3], int[3]> ();
    test_remove_extent<const int[][3], const int[3]> ();
    test_remove_extent<int[2][3], int[3]> ();
    test_remove_extent<const int[2][3], const int[3]> ();
    test_remove_extent<int[1][2][3], int[2][3]> ();
    test_remove_extent<const int[1][2][3], const int[2][3]> ();
}
