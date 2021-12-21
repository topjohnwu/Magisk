//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ostream>

// template <class charT, class traits = char_traits<charT> >
// class basic_ostream::sentry;

// ~sentry();

#include <ostream>
#include <cassert>

#include "test_macros.h"

int sync_called = 0;

template <class CharT>
struct testbuf1
    : public std::basic_streambuf<CharT>
{
    testbuf1() {}

protected:

    int virtual sync()
    {
        ++sync_called;
        return 1;
    }
};

int main()
{
    {
        std::ostream os((std::streambuf*)0);
        std::ostream::sentry s(os);
        assert(!bool(s));
    }
    assert(sync_called == 0);
    {
        testbuf1<char> sb;
        std::ostream os(&sb);
        std::ostream::sentry s(os);
        assert(bool(s));
    }
    assert(sync_called == 0);
    {
        testbuf1<char> sb;
        std::ostream os(&sb);
        std::ostream::sentry s(os);
        assert(bool(s));
        unitbuf(os);
    }
    assert(sync_called == 1);
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        testbuf1<char> sb;
        std::ostream os(&sb);
        try
        {
            std::ostream::sentry s(os);
            assert(bool(s));
            unitbuf(os);
            throw 1;
        }
        catch (...)
        {
        }
        assert(sync_called == 1);
    }
#endif
}
