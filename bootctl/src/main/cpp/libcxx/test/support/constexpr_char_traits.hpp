// -*- C++ -*-
//===-------------------- constexpr_char_traits ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _CONSTEXPR_CHAR_TRAITS
#define _CONSTEXPR_CHAR_TRAITS

#include <string>
#include <cassert>

#include "test_macros.h"

template <class _CharT>
struct constexpr_char_traits
{
    typedef _CharT    char_type;
    typedef int       int_type;
    typedef std::streamoff off_type;
    typedef std::streampos pos_type;
    typedef std::mbstate_t state_type;

    static TEST_CONSTEXPR_CXX14 void assign(char_type& __c1, const char_type& __c2) TEST_NOEXCEPT
        {__c1 = __c2;}

    static TEST_CONSTEXPR bool eq(char_type __c1, char_type __c2) TEST_NOEXCEPT
        {return __c1 == __c2;}

    static TEST_CONSTEXPR  bool lt(char_type __c1, char_type __c2) TEST_NOEXCEPT
        {return __c1 < __c2;}

    static TEST_CONSTEXPR_CXX14 int              compare(const char_type* __s1, const char_type* __s2, size_t __n);
    static TEST_CONSTEXPR_CXX14 size_t           length(const char_type* __s);
    static TEST_CONSTEXPR_CXX14 const char_type* find(const char_type* __s, size_t __n, const char_type& __a);
    static TEST_CONSTEXPR_CXX14 char_type*       move(char_type* __s1, const char_type* __s2, size_t __n);
    static TEST_CONSTEXPR_CXX14 char_type*       copy(char_type* __s1, const char_type* __s2, size_t __n);
    static TEST_CONSTEXPR_CXX14 char_type*       assign(char_type* __s, size_t __n, char_type __a);

    static TEST_CONSTEXPR int_type  not_eof(int_type __c) TEST_NOEXCEPT
        {return eq_int_type(__c, eof()) ? ~eof() : __c;}

    static TEST_CONSTEXPR char_type to_char_type(int_type __c) TEST_NOEXCEPT
        {return char_type(__c);}

    static TEST_CONSTEXPR int_type  to_int_type(char_type __c) TEST_NOEXCEPT
        {return int_type(__c);}

    static TEST_CONSTEXPR bool      eq_int_type(int_type __c1, int_type __c2) TEST_NOEXCEPT
        {return __c1 == __c2;}

    static TEST_CONSTEXPR int_type  eof() TEST_NOEXCEPT
        {return int_type(EOF);}
};


template <class _CharT>
TEST_CONSTEXPR_CXX14 int
constexpr_char_traits<_CharT>::compare(const char_type* __s1, const char_type* __s2, size_t __n)
{
    for (; __n; --__n, ++__s1, ++__s2)
    {
        if (lt(*__s1, *__s2))
            return -1;
        if (lt(*__s2, *__s1))
            return 1;
    }
    return 0;
}

template <class _CharT>
TEST_CONSTEXPR_CXX14 size_t
constexpr_char_traits<_CharT>::length(const char_type* __s)
{
    size_t __len = 0;
    for (; !eq(*__s, char_type(0)); ++__s)
        ++__len;
    return __len;
}

template <class _CharT>
TEST_CONSTEXPR_CXX14 const _CharT*
constexpr_char_traits<_CharT>::find(const char_type* __s, size_t __n, const char_type& __a)
{
    for (; __n; --__n)
    {
        if (eq(*__s, __a))
            return __s;
        ++__s;
    }
    return 0;
}

template <class _CharT>
TEST_CONSTEXPR_CXX14 _CharT*
constexpr_char_traits<_CharT>::move(char_type* __s1, const char_type* __s2, size_t __n)
{
    char_type* __r = __s1;
    if (__s1 < __s2)
    {
        for (; __n; --__n, ++__s1, ++__s2)
            assign(*__s1, *__s2);
    }
    else if (__s2 < __s1)
    {
        __s1 += __n;
        __s2 += __n;
        for (; __n; --__n)
            assign(*--__s1, *--__s2);
    }
    return __r;
}

template <class _CharT>
TEST_CONSTEXPR_CXX14 _CharT*
constexpr_char_traits<_CharT>::copy(char_type* __s1, const char_type* __s2, size_t __n)
{
    assert(__s2 < __s1 || __s2 >= __s1+__n);
    char_type* __r = __s1;
    for (; __n; --__n, ++__s1, ++__s2)
        assign(*__s1, *__s2);
    return __r;
}

template <class _CharT>
TEST_CONSTEXPR_CXX14 _CharT*
constexpr_char_traits<_CharT>::assign(char_type* __s, size_t __n, char_type __a)
{
    char_type* __r = __s;
    for (; __n; --__n, ++__s)
        assign(*__s, __a);
    return __r;
}

#endif // _CONSTEXPR_CHAR_TRAITS
