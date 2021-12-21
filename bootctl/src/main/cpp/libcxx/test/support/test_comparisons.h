//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//  A set of routines for testing the comparison operators of a type
//
//      XXXX6 tests all six comparison operators
//      XXXX2 tests only op== and op!=
//
//      AssertComparisonsXAreNoexcept       static_asserts that the operations are all noexcept.
//      AssertComparisonsXReturnBool        static_asserts that the operations return bool.
//      AssertComparisonsXConvertibleToBool static_asserts that the operations return something convertible to bool.


#ifndef TEST_COMPARISONS_H
#define TEST_COMPARISONS_H

#include <type_traits>
#include "test_macros.h"

//  Test all six comparison operations for sanity
template <class T>
TEST_CONSTEXPR_CXX14 bool testComparisons6(const T& t1, const T& t2, bool isEqual, bool isLess)
{
    if (isEqual)
        {
        if (!(t1 == t2)) return false;
        if (!(t2 == t1)) return false;
        if ( (t1 != t2)) return false;
        if ( (t2 != t1)) return false;
        if ( (t1  < t2)) return false;
        if ( (t2  < t1)) return false;
        if (!(t1 <= t2)) return false;
        if (!(t2 <= t1)) return false;
        if ( (t1  > t2)) return false;
        if ( (t2  > t1)) return false;
        if (!(t1 >= t2)) return false;
        if (!(t2 >= t1)) return false;
        }
    else if (isLess)
        {
        if ( (t1 == t2)) return false;
        if ( (t2 == t1)) return false;
        if (!(t1 != t2)) return false;
        if (!(t2 != t1)) return false;
        if (!(t1  < t2)) return false;
        if ( (t2  < t1)) return false;
        if (!(t1 <= t2)) return false;
        if ( (t2 <= t1)) return false;
        if ( (t1  > t2)) return false;
        if (!(t2  > t1)) return false;
        if ( (t1 >= t2)) return false;
        if (!(t2 >= t1)) return false;
        }
    else /* greater */
        {
        if ( (t1 == t2)) return false;
        if ( (t2 == t1)) return false;
        if (!(t1 != t2)) return false;
        if (!(t2 != t1)) return false;
        if ( (t1  < t2)) return false;
        if (!(t2  < t1)) return false;
        if ( (t1 <= t2)) return false;
        if (!(t2 <= t1)) return false;
        if (!(t1  > t2)) return false;
        if ( (t2  > t1)) return false;
        if (!(t1 >= t2)) return false;
        if ( (t2 >= t1)) return false;
        }

    return true;
}

//  Easy call when you can init from something already comparable.
template <class T, class Param>
TEST_CONSTEXPR_CXX14 bool testComparisons6Values(Param val1, Param val2)
{
    const bool isEqual = val1 == val2;
    const bool isLess  = val1  < val2;

    return testComparisons6(T(val1), T(val2), isEqual, isLess);
}

template <class T>
void AssertComparisons6AreNoexcept()
{
    ASSERT_NOEXCEPT(std::declval<const T&>() == std::declval<const T&>());
    ASSERT_NOEXCEPT(std::declval<const T&>() != std::declval<const T&>());
    ASSERT_NOEXCEPT(std::declval<const T&>() <  std::declval<const T&>());
    ASSERT_NOEXCEPT(std::declval<const T&>() <= std::declval<const T&>());
    ASSERT_NOEXCEPT(std::declval<const T&>() >  std::declval<const T&>());
    ASSERT_NOEXCEPT(std::declval<const T&>() >= std::declval<const T&>());
}

template <class T>
void AssertComparisons6ReturnBool()
{
    ASSERT_SAME_TYPE(decltype(std::declval<const T&>() == std::declval<const T&>()), bool);
    ASSERT_SAME_TYPE(decltype(std::declval<const T&>() != std::declval<const T&>()), bool);
    ASSERT_SAME_TYPE(decltype(std::declval<const T&>() <  std::declval<const T&>()), bool);
    ASSERT_SAME_TYPE(decltype(std::declval<const T&>() <= std::declval<const T&>()), bool);
    ASSERT_SAME_TYPE(decltype(std::declval<const T&>() >  std::declval<const T&>()), bool);
    ASSERT_SAME_TYPE(decltype(std::declval<const T&>() >= std::declval<const T&>()), bool);
}


template <class T>
void AssertComparisons6ConvertibleToBool()
{
    static_assert((std::is_convertible<decltype(std::declval<const T&>() == std::declval<const T&>()), bool>::value), "");
    static_assert((std::is_convertible<decltype(std::declval<const T&>() != std::declval<const T&>()), bool>::value), "");
    static_assert((std::is_convertible<decltype(std::declval<const T&>() <  std::declval<const T&>()), bool>::value), "");
    static_assert((std::is_convertible<decltype(std::declval<const T&>() <= std::declval<const T&>()), bool>::value), "");
    static_assert((std::is_convertible<decltype(std::declval<const T&>() >  std::declval<const T&>()), bool>::value), "");
    static_assert((std::is_convertible<decltype(std::declval<const T&>() >= std::declval<const T&>()), bool>::value), "");
}

//  Test all six comparison operations for sanity
template <class T>
TEST_CONSTEXPR_CXX14 bool testComparisons2(const T& t1, const T& t2, bool isEqual)
{
    if (isEqual)
        {
        if (!(t1 == t2)) return false;
        if (!(t2 == t1)) return false;
        if ( (t1 != t2)) return false;
        if ( (t2 != t1)) return false;
        }
    else /* greater */
        {
        if ( (t1 == t2)) return false;
        if ( (t2 == t1)) return false;
        if (!(t1 != t2)) return false;
        if (!(t2 != t1)) return false;
        }

    return true;
}

//  Easy call when you can init from something already comparable.
template <class T, class Param>
TEST_CONSTEXPR_CXX14 bool testComparisons2Values(Param val1, Param val2)
{
    const bool isEqual = val1 == val2;

    return testComparisons2(T(val1), T(val2), isEqual);
}

template <class T>
void AssertComparisons2AreNoexcept()
{
    ASSERT_NOEXCEPT(std::declval<const T&>() == std::declval<const T&>());
    ASSERT_NOEXCEPT(std::declval<const T&>() != std::declval<const T&>());
}

template <class T>
void AssertComparisons2ReturnBool()
{
    ASSERT_SAME_TYPE(decltype(std::declval<const T&>() == std::declval<const T&>()), bool);
    ASSERT_SAME_TYPE(decltype(std::declval<const T&>() != std::declval<const T&>()), bool);
}


template <class T>
void AssertComparisons2ConvertibleToBool()
{
    static_assert((std::is_convertible<decltype(std::declval<const T&>() == std::declval<const T&>()), bool>::value), "");
    static_assert((std::is_convertible<decltype(std::declval<const T&>() != std::declval<const T&>()), bool>::value), "");
}

#endif // TEST_COMPARISONS_H
