//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iomanip>

// T3 setbase(int base);

#include <iomanip>
#include <istream>
#include <ostream>
#include <cassert>

template <class CharT>
struct testbuf
    : public std::basic_streambuf<CharT>
{
    testbuf() {}
};

int main()
{
    {
        testbuf<char> sb;
        std::istream is(&sb);
        is >> std::setbase(8);
        assert((is.flags() & std::ios_base::basefield) == std::ios_base::oct);
        is >> std::setbase(10);
        assert((is.flags() & std::ios_base::basefield) == std::ios_base::dec);
        is >> std::setbase(16);
        assert((is.flags() & std::ios_base::basefield) == std::ios_base::hex);
        is >> std::setbase(15);
        assert((is.flags() & std::ios_base::basefield) == 0);
    }
    {
        testbuf<char> sb;
        std::ostream os(&sb);
        os << std::setbase(8);
        assert((os.flags() & std::ios_base::basefield) == std::ios_base::oct);
        os << std::setbase(10);
        assert((os.flags() & std::ios_base::basefield) == std::ios_base::dec);
        os << std::setbase(16);
        assert((os.flags() & std::ios_base::basefield) == std::ios_base::hex);
        os << std::setbase(15);
        assert((os.flags() & std::ios_base::basefield) == 0);
    }
    {
        testbuf<wchar_t> sb;
        std::wistream is(&sb);
        is >> std::setbase(8);
        assert((is.flags() & std::ios_base::basefield) == std::ios_base::oct);
        is >> std::setbase(10);
        assert((is.flags() & std::ios_base::basefield) == std::ios_base::dec);
        is >> std::setbase(16);
        assert((is.flags() & std::ios_base::basefield) == std::ios_base::hex);
        is >> std::setbase(15);
        assert((is.flags() & std::ios_base::basefield) == 0);
    }
    {
        testbuf<wchar_t> sb;
        std::wostream os(&sb);
        os << std::setbase(8);
        assert((os.flags() & std::ios_base::basefield) == std::ios_base::oct);
        os << std::setbase(10);
        assert((os.flags() & std::ios_base::basefield) == std::ios_base::dec);
        os << std::setbase(16);
        assert((os.flags() & std::ios_base::basefield) == std::ios_base::hex);
        os << std::setbase(15);
        assert((os.flags() & std::ios_base::basefield) == 0);
    }
}
