//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// class ostream_iterator

// ostream_iterator(ostream_type& s);

#include <iterator>
#include <sstream>
#include <cassert>

struct MyTraits : std::char_traits<char> {};

typedef std::basic_ostringstream<char, MyTraits> StringStream;
typedef std::basic_ostream<char, MyTraits> BasicStream;

void operator&(BasicStream const&) {}

int main()
{
    {
        std::ostringstream outf;
        std::ostream_iterator<int> i(outf);
        assert(outf.good());
    }
    {
        StringStream outf;
        std::ostream_iterator<int, char, MyTraits> i(outf);
        assert(outf.good());
    }
}
