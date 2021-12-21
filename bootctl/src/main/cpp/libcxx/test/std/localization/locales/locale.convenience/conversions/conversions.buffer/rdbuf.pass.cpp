//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// wbuffer_convert<Codecvt, Elem, Tr>

// streambuf *rdbuf(streambuf *bytebuf);

#include <locale>
#include <codecvt>
#include <sstream>
#include <cassert>

int main()
{
    typedef std::wbuffer_convert<std::codecvt_utf8<wchar_t> > B;
    {
        std::stringstream s;
        B b;
        assert(b.rdbuf() == nullptr);
        b.rdbuf(s.rdbuf());
        assert(b.rdbuf() == s.rdbuf());
    }
}
