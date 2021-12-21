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
//   class basic_ostream;

// pos_type tellp();

#include <ostream>
#include <cassert>

int seekoff_called = 0;

template <class CharT>
struct testbuf
    : public std::basic_streambuf<CharT>
{
    typedef std::basic_streambuf<CharT> base;
    testbuf() {}

protected:

    typename base::pos_type
    seekoff(typename base::off_type off, std::ios_base::seekdir way, std::ios_base::openmode which)
    {
        assert(off == 0);
        assert(way == std::ios_base::cur);
        assert(which == std::ios_base::out);
        ++seekoff_called;
        return 10;
    }
};

int main()
{
    {
        std::ostream os((std::streambuf*)0);
        assert(os.tellp() == -1);
    }
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        assert(os.tellp() == 10);
        assert(seekoff_called == 1);
    }
}
