//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// template <class charT, class traits> class basic_ios

// void clear(iostate state = goodbit);

#include <ios>
#include <streambuf>
#include <cassert>

#include "test_macros.h"

struct testbuf : public std::streambuf {};

int main()
{
    {
        std::ios ios(0);
        ios.clear();
        assert(ios.rdstate() == std::ios::badbit);
#ifndef TEST_HAS_NO_EXCEPTIONS
        try
        {
            ios.exceptions(std::ios::badbit);
            assert(false);
        }
        catch (...)
        {
        }
        try
        {
            ios.clear();
            assert(false);
        }
        catch (std::ios::failure&)
        {
            assert(ios.rdstate() == std::ios::badbit);
        }
        try
        {
            ios.clear(std::ios::eofbit);
            assert(false);
        }
        catch (std::ios::failure&)
        {
            assert(ios.rdstate() == (std::ios::eofbit | std::ios::badbit));
        }
#endif
    }
    {
        testbuf sb;
        std::ios ios(&sb);
        ios.clear();
        assert(ios.rdstate() == std::ios::goodbit);
        ios.exceptions(std::ios::badbit);
        ios.clear();
        assert(ios.rdstate() == std::ios::goodbit);
        ios.clear(std::ios::eofbit);
        assert(ios.rdstate() == std::ios::eofbit);
    }
}
