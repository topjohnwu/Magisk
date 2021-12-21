//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <istream>

// template <class charT, class traits = char_traits<charT> >
//   class basic_istream;

// basic_istream<charT,traits>& operator>>(ios_base& (*pf)(ios_base&));

#include <istream>
#include <cassert>

int f_called = 0;

std::ios_base&
f(std::ios_base& is)
{
    ++f_called;
    return is;
}

int main()
{
    {
        std::istream is((std::streambuf*)0);
        is >> f;
        assert(f_called == 1);
    }
}
