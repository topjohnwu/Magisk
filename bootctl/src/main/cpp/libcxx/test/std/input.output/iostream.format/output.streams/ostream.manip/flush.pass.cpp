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

// template <class charT, class traits>
//   basic_ostream<charT,traits>& flush(basic_ostream<charT,traits>& os);

#include <ostream>
#include <cassert>

int sync_called = 0;

template <class CharT>
class testbuf
    : public std::basic_streambuf<CharT>
{
public:
    testbuf()
    {
    }

protected:

    virtual int
        sync()
        {
            ++sync_called;
            return 0;
        }
};

int main()
{
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        flush(os);
        assert(sync_called == 1);
        assert(os.good());
    }
    {
        testbuf<wchar_t> sb;
        std::wostream os(&sb);
        flush(os);
        assert(sync_called == 2);
        assert(os.good());
    }
}
