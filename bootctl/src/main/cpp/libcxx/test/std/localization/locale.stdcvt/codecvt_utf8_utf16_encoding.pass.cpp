//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <codecvt>

// template <class Elem, unsigned long Maxcode = 0x10ffff,
//           codecvt_mode Mode = (codecvt_mode)0>
// class codecvt_utf8_utf16
//     : public codecvt<Elem, char, mbstate_t>
// {
//     // unspecified
// };

// int encoding() const throw();

#include <codecvt>
#include <cassert>

int main()
{
    {
        typedef std::codecvt_utf8_utf16<wchar_t> C;
        C c;
        int r = c.encoding();
        assert(r == 0);
    }
    {
        typedef std::codecvt_utf8_utf16<char16_t> C;
        C c;
        int r = c.encoding();
        assert(r == 0);
    }
    {
        typedef std::codecvt_utf8_utf16<char32_t> C;
        C c;
        int r = c.encoding();
        assert(r == 0);
    }
}
