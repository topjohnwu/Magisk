//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test:

// template <class charT, class traits, class Allocator>
// basic_string<charT, traits, Allocator>
// to_string(charT zero = charT('0'), charT one = charT('1')) const;
//
// template <class charT, class traits>
// basic_string<charT, traits, allocator<charT> > to_string() const;
//
// template <class charT>
// basic_string<charT, char_traits<charT>, allocator<charT> > to_string() const;
//
// basic_string<char, char_traits<char>, allocator<char> > to_string() const;

#include <bitset>
#include <string>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

#if defined(TEST_COMPILER_CLANG)
#pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(TEST_COMPILER_C1XX)
#pragma warning(disable: 6294) // Ill-defined for-loop:  initial condition does not satisfy test.  Loop body not executed.
#endif

template <std::size_t N>
std::bitset<N>
make_bitset()
{
    std::bitset<N> v;
    for (std::size_t i = 0; i < N; ++i)
        v[i] = static_cast<bool>(std::rand() & 1);
    return v;
}

template <std::size_t N>
void test_to_string()
{
{
    std::bitset<N> v = make_bitset<N>();
    {
    std::wstring s = v.template to_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >();
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
    {
    std::wstring s = v.template to_string<wchar_t, std::char_traits<wchar_t> >();
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
    {
    std::string s = v.template to_string<char>();
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
    {
    std::string s = v.to_string();
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
}
{
    std::bitset<N> v = make_bitset<N>();
    {
    std::wstring s = v.template to_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >('0');
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
    {
    std::wstring s = v.template to_string<wchar_t, std::char_traits<wchar_t> >('0');
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
    {
    std::string s = v.template to_string<char>('0');
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
    {
    std::string s = v.to_string('0');
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
}
{
    std::bitset<N> v = make_bitset<N>();
    {
    std::wstring s = v.template to_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >('0', '1');
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
    {
    std::wstring s = v.template to_string<wchar_t, std::char_traits<wchar_t> >('0', '1');
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
    {
    std::string s = v.template to_string<char>('0', '1');
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
    {
    std::string s = v.to_string('0', '1');
    for (std::size_t i = 0; i < N; ++i)
        if (v[i])
            assert(s[N - 1 - i] == '1');
        else
            assert(s[N - 1 - i] == '0');
    }
}
}

int main()
{
    test_to_string<0>();
    test_to_string<1>();
    test_to_string<31>();
    test_to_string<32>();
    test_to_string<33>();
    test_to_string<63>();
    test_to_string<64>();
    test_to_string<65>();
    test_to_string<1000>();
}
