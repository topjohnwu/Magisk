//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>

// wstring_convert(Codecvt* pcvt, state_type state);

#include <locale>
#include <codecvt>
#include <cassert>

int main()
{
    {
        typedef std::codecvt_utf8<wchar_t> Codecvt;
        typedef std::wstring_convert<Codecvt> Myconv;
        Myconv myconv(new Codecvt, std::mbstate_t());
        assert(myconv.converted() == 0);
    }
}
