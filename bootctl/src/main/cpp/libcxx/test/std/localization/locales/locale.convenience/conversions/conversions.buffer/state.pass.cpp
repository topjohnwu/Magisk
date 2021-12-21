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

// state_type state() const;

#include <locale>
#include <codecvt>
#include <sstream>
#include <cassert>

int main()
{
    typedef std::wbuffer_convert<std::codecvt_utf8<wchar_t> > B;
    {
        B b;
        std::mbstate_t s = b.state();
        ((void)s);
    }
}
