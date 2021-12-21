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

// basic_ostream<charT,traits>& seekp(pos_type pos);

#include <ostream>
#include <cassert>

int seekpos_called = 0;

template <class CharT>
struct testbuf
    : public std::basic_streambuf<CharT>
{
    typedef std::basic_streambuf<CharT> base;
    testbuf() {}

protected:

    typename base::pos_type
    seekpos(typename base::pos_type sp, std::ios_base::openmode which)
    {
        ++seekpos_called;
        assert(which == std::ios_base::out);
        return sp;
    }
};

int main()
{
    {
        seekpos_called = 0;
        std::ostream os((std::streambuf*)0);
        assert(&os.seekp(5) == &os);
        assert(seekpos_called == 0);
    }
    {
        seekpos_called = 0;
        testbuf<char> sb;
        std::ostream os(&sb);
        assert(&os.seekp(10) == &os);
        assert(seekpos_called == 1);
        assert(os.good());
        assert(&os.seekp(-1) == &os);
        assert(seekpos_called == 2);
        assert(os.fail());
    }
    { // See https://bugs.llvm.org/show_bug.cgi?id=21361
        seekpos_called = 0;
        testbuf<char> sb;
        std::ostream os(&sb);
        os.setstate(std::ios_base::eofbit);
        assert(&os.seekp(10) == &os);
        assert(seekpos_called == 1);
        assert(os.rdstate() == std::ios_base::eofbit);
    }
}
