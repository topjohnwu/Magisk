//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// class ostreambuf_iterator

// ostreambuf_iterator(streambuf_type* s) throw();

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    {
        std::ostringstream outf;
        std::ostreambuf_iterator<char> i(outf.rdbuf());
        assert(!i.failed());
    }
    {
        std::wostringstream outf;
        std::ostreambuf_iterator<wchar_t> i(outf.rdbuf());
        assert(!i.failed());
    }
}
