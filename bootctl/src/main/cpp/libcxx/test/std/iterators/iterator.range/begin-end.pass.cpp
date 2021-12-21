//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// XFAIL: c++98, c++03

// <iterator>
// template <class C> constexpr auto begin(C& c) -> decltype(c.begin());
// template <class C> constexpr auto begin(const C& c) -> decltype(c.begin());
// template <class C> constexpr auto cbegin(const C& c) -> decltype(std::begin(c)); // C++14
// template <class C> constexpr auto cend(const C& c) -> decltype(std::end(c));     // C++14
// template <class C> constexpr auto end  (C& c) -> decltype(c.end());
// template <class C> constexpr auto end  (const C& c) -> decltype(c.end());
// template <class E> constexpr reverse_iterator<const E*> rbegin(initializer_list<E> il);
// template <class E> constexpr reverse_iterator<const E*> rend  (initializer_list<E> il);
//
// template <class C> auto constexpr rbegin(C& c) -> decltype(c.rbegin());                 // C++14
// template <class C> auto constexpr rbegin(const C& c) -> decltype(c.rbegin());           // C++14
// template <class C> auto constexpr rend(C& c) -> decltype(c.rend());                     // C++14
// template <class C> constexpr auto rend(const C& c) -> decltype(c.rend());               // C++14
// template <class T, size_t N> reverse_iterator<T*> constexpr rbegin(T (&array)[N]);      // C++14
// template <class T, size_t N> reverse_iterator<T*> constexpr rend(T (&array)[N]);        // C++14
// template <class C> constexpr auto crbegin(const C& c) -> decltype(std::rbegin(c));      // C++14
// template <class C> constexpr auto crend(const C& c) -> decltype(std::rend(c));          // C++14
//
//  All of these are constexpr in C++17

#include "test_macros.h"

#include <iterator>
#include <cassert>
#include <vector>
#include <array>
#include <list>
#include <initializer_list>

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

template<typename C>
void test_const_container( const C & c, typename C::value_type val ) {
    assert ( std::begin(c)   == c.begin());
    assert (*std::begin(c)   ==  val );
    assert ( std::begin(c)   != c.end());
    assert ( std::end(c)     == c.end());
#if TEST_STD_VER > 11
    assert ( std::cbegin(c)  == c.cbegin());
    assert ( std::cbegin(c)  != c.cend());
    assert ( std::cend(c)    == c.cend());
    assert ( std::rbegin(c)  == c.rbegin());
    assert ( std::rbegin(c)  != c.rend());
    assert ( std::rend(c)    == c.rend());
    assert ( std::crbegin(c) == c.crbegin());
    assert ( std::crbegin(c) != c.crend());
    assert ( std::crend(c)   == c.crend());
#endif
    }

template<typename T>
void test_const_container( const std::initializer_list<T> & c, T val ) {
    assert ( std::begin(c)   == c.begin());
    assert (*std::begin(c)   ==  val );
    assert ( std::begin(c)   != c.end());
    assert ( std::end(c)     == c.end());
#if TEST_STD_VER > 11
//  initializer_list doesn't have cbegin/cend/rbegin/rend
//  but std::cbegin(),etc work (b/c they're general fn templates)
//     assert ( std::cbegin(c)  == c.cbegin());
//     assert ( std::cbegin(c)  != c.cend());
//     assert ( std::cend(c)    == c.cend());
//     assert ( std::rbegin(c)  == c.rbegin());
//     assert ( std::rbegin(c)  != c.rend());
//     assert ( std::rend(c)    == c.rend());
//     assert ( std::crbegin(c) == c.crbegin());
//     assert ( std::crbegin(c) != c.crend());
//     assert ( std::crend(c)   == c.crend());
#endif
    }

template<typename C>
void test_container( C & c, typename C::value_type val ) {
    assert ( std::begin(c)   == c.begin());
    assert (*std::begin(c)   ==  val );
    assert ( std::begin(c)   != c.end());
    assert ( std::end(c)     == c.end());
#if TEST_STD_VER > 11
    assert ( std::cbegin(c)  == c.cbegin());
    assert ( std::cbegin(c)  != c.cend());
    assert ( std::cend(c)    == c.cend());
    assert ( std::rbegin(c)  == c.rbegin());
    assert ( std::rbegin(c)  != c.rend());
    assert ( std::rend(c)    == c.rend());
    assert ( std::crbegin(c) == c.crbegin());
    assert ( std::crbegin(c) != c.crend());
    assert ( std::crend(c)   == c.crend());
#endif
    }

template<typename T>
void test_container( std::initializer_list<T> & c, T val ) {
    assert ( std::begin(c)   == c.begin());
    assert (*std::begin(c)   ==  val );
    assert ( std::begin(c)   != c.end());
    assert ( std::end(c)     == c.end());
#if TEST_STD_VER > 11
//  initializer_list doesn't have cbegin/cend/rbegin/rend
//     assert ( std::cbegin(c)  == c.cbegin());
//     assert ( std::cbegin(c)  != c.cend());
//     assert ( std::cend(c)    == c.cend());
//     assert ( std::rbegin(c)  == c.rbegin());
//     assert ( std::rbegin(c)  != c.rend());
//     assert ( std::rend(c)    == c.rend());
//     assert ( std::crbegin(c) == c.crbegin());
//     assert ( std::crbegin(c) != c.crend());
//     assert ( std::crend(c)   == c.crend());
#endif
    }

template<typename T, size_t Sz>
void test_const_array( const T (&array)[Sz] ) {
    assert ( std::begin(array)  == array );
    assert (*std::begin(array)  ==  array[0] );
    assert ( std::begin(array)  != std::end(array));
    assert ( std::end(array)    == array + Sz);
#if TEST_STD_VER > 11
    assert ( std::cbegin(array) == array );
    assert (*std::cbegin(array) == array[0] );
    assert ( std::cbegin(array) != std::cend(array));
    assert ( std::cend(array)   == array + Sz);
#endif
    }

int main(){
    std::vector<int> v; v.push_back(1);
    std::list<int> l;   l.push_back(2);
    std::array<int, 1> a; a[0] = 3;
    std::initializer_list<int> il = { 4 };

    test_container ( v, 1 );
    test_container ( l, 2 );
    test_container ( a, 3 );
    test_container ( il, 4 );

    test_const_container ( v, 1 );
    test_const_container ( l, 2 );
    test_const_container ( a, 3 );
    test_const_container ( il, 4 );

    static constexpr int arrA [] { 1, 2, 3 };
    test_const_array ( arrA );
#if TEST_STD_VER > 11
    constexpr const int *b = std::cbegin(arrA);
    constexpr const int *e = std::cend(arrA);
    static_assert(e - b == 3, "");
#endif

#if TEST_STD_VER > 14
    {
        typedef std::array<int, 5> C;
        constexpr const C c{0,1,2,3,4};

        static_assert ( c.begin()   == std::begin(c), "");
        static_assert ( c.cbegin()  == std::cbegin(c), "");
        static_assert ( c.end()     == std::end(c), "");
        static_assert ( c.cend()    == std::cend(c), "");

        static_assert ( c.rbegin()  == std::rbegin(c), "");
        static_assert ( c.crbegin() == std::crbegin(c), "");
        static_assert ( c.rend()    == std::rend(c), "");
        static_assert ( c.crend()   == std::crend(c), "");

        static_assert ( std::begin(c)   != std::end(c), "");
        static_assert ( std::rbegin(c)  != std::rend(c), "");
        static_assert ( std::cbegin(c)  != std::cend(c), "");
        static_assert ( std::crbegin(c) != std::crend(c), "");

        static_assert ( *c.begin()  == 0, "");
        static_assert ( *c.rbegin()  == 4, "");

        static_assert ( *std::begin(c)   == 0, "" );
        static_assert ( *std::cbegin(c)  == 0, "" );
        static_assert ( *std::rbegin(c)  == 4, "" );
        static_assert ( *std::crbegin(c) == 4, "" );
    }

    {
        static constexpr const int c[] = {0,1,2,3,4};

        static_assert ( *std::begin(c)   == 0, "" );
        static_assert ( *std::cbegin(c)  == 0, "" );
        static_assert ( *std::rbegin(c)  == 4, "" );
        static_assert ( *std::crbegin(c) == 4, "" );
    }
#endif
}
