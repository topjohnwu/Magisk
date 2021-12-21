//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// remove_all_extents

#include <type_traits>

#include "test_macros.h"

enum Enum {zero, one_};

template <class T, class U>
void test_remove_all_extents()
{
    static_assert((std::is_same<typename std::remove_all_extents<T>::type, U>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::remove_all_extents_t<T>,     U>::value), "");
#endif
}

int main()
{
    test_remove_all_extents<int, int> ();
    test_remove_all_extents<const Enum, const Enum> ();
    test_remove_all_extents<int[], int> ();
    test_remove_all_extents<const int[], const int> ();
    test_remove_all_extents<int[3], int> ();
    test_remove_all_extents<const int[3], const int> ();
    test_remove_all_extents<int[][3], int> ();
    test_remove_all_extents<const int[][3], const int> ();
    test_remove_all_extents<int[2][3], int> ();
    test_remove_all_extents<const int[2][3], const int> ();
    test_remove_all_extents<int[1][2][3], int> ();
    test_remove_all_extents<const int[1][2][3], const int> ();
}
