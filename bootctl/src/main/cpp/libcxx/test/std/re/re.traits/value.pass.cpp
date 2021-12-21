//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT> struct regex_traits;

// int value(charT ch, int radix) const;

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    {
        std::regex_traits<char> t;

        for (char c = 0; c < '0'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == -1);
        }
        for (char c = '0'; c < '8'; ++c)
        {
            assert(t.value(c, 8) == c - '0');
            assert(t.value(c, 10) == c - '0');
            assert(t.value(c, 16) == c - '0');
        }
        for (char c = '8'; c < ':'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == c - '0');
            assert(t.value(c, 16) == c - '0');
        }
        for (char c = ':'; c < 'A'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == -1);
        }
        for (char c = 'A'; c < 'G'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == c - 'A' +10);
        }
        for (char c = 'G'; c < 'a'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == -1);
        }
        for (char c = 'a'; c < 'g'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == c - 'a' +10);
        }
        for (int c = 'g'; c < 256; ++c)
        {
            assert(t.value(char(c), 8) == -1);
            assert(t.value(char(c), 10) == -1);
            assert(t.value(char(c), 16) == -1);
        }
    }
    {
        std::regex_traits<wchar_t> t;

        for (wchar_t c = 0; c < '0'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == -1);
        }
        for (wchar_t c = '0'; c < '8'; ++c)
        {
            assert(t.value(c, 8) ==  static_cast<int>(c - '0'));
            assert(t.value(c, 10) == static_cast<int>(c - '0'));
            assert(t.value(c, 16) == static_cast<int>(c - '0'));
        }
        for (wchar_t c = '8'; c < ':'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == static_cast<int>(c - '0'));
            assert(t.value(c, 16) == static_cast<int>(c - '0'));
        }
        for (wchar_t c = ':'; c < 'A'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == -1);
        }
        for (wchar_t c = 'A'; c < 'G'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == static_cast<int>(c - 'A' +10));
        }
        for (wchar_t c = 'G'; c < 'a'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == -1);
        }
        for (wchar_t c = 'a'; c < 'g'; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == static_cast<int>(c - 'a' +10));
        }
        for (wchar_t c = 'g'; c < 0xFFFF; ++c)
        {
            assert(t.value(c, 8) == -1);
            assert(t.value(c, 10) == -1);
            assert(t.value(c, 16) == -1);
        }
    }
}
