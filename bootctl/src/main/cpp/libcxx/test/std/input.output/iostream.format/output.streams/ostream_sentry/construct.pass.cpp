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

// explicit sentry(basic_ostream<charT,traits>& os);

#include <ostream>
#include <cassert>

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
    {
        testbuf1<char> sb;
        std::ostream os(&sb);
        std::ostream::sentry s(os);
        assert(bool(s));
    }
    {
        testbuf1<char> sb;
        std::ostream os(&sb);
        testbuf1<char> sb2;
        std::ostream os2(&sb2);
        os.tie(&os2);
        assert(sync_called == 0);
        std::ostream::sentry s(os);
        assert(bool(s));
        assert(sync_called == 1);
    }
}
