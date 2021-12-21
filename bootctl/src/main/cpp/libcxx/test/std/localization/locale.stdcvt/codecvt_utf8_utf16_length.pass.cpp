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

// int length(stateT& state, const externT* from, const externT* from_end,
//            size_t max) const;

#include <codecvt>
#include <cassert>

int main()
{
    {
        typedef std::codecvt_utf8_utf16<wchar_t> C;
        C c;
        char n[4] = {char(0xF1), char(0x80), char(0x80), char(0x83)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 4);

        n[0] = char(0xE1);
        n[1] = char(0x80);
        n[2] = char(0x85);
        r = c.length(m, n, n+3, 2);
        assert(r == 3);

        n[0] = char(0xD1);
        n[1] = char(0x93);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x56);
        r = c.length(m, n, n+1, 2);
        assert(r == 1);
    }
    {
        typedef std::codecvt_utf8_utf16<wchar_t, 0x1000> C;
        C c;
        char n[4] = {char(0xF1), char(0x80), char(0x80), char(0x83)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[0] = char(0xE1);
        n[1] = char(0x80);
        n[2] = char(0x85);
        r = c.length(m, n, n+3, 2);
        assert(r == 0);

        n[0] = char(0xD1);
        n[1] = char(0x93);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x56);
        r = c.length(m, n, n+1, 2);
        assert(r == 1);
    }
    {
        typedef std::codecvt_utf8_utf16<wchar_t, 0x10ffff, std::consume_header> C;
        C c;
        char n[7] = {char(0xEF), char(0xBB), char(0xBF), char(0xF1), char(0x80), char(0x80), char(0x83)};
        std::mbstate_t m;
        int r = c.length(m, n, n+7, 2);
        assert(r == 7);

        n[0] = char(0xE1);
        n[1] = char(0x80);
        n[2] = char(0x85);
        r = c.length(m, n, n+3, 2);
        assert(r == 3);

        n[0] = char(0xD1);
        n[1] = char(0x93);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x56);
        r = c.length(m, n, n+1, 2);
        assert(r == 1);
    }
    {
        typedef std::codecvt_utf8_utf16<char32_t> C;
        C c;
        char n[4] = {char(0xF1), char(0x80), char(0x80), char(0x83)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 4);

        n[0] = char(0xE1);
        n[1] = char(0x80);
        n[2] = char(0x85);
        r = c.length(m, n, n+3, 2);
        assert(r == 3);

        n[0] = char(0xD1);
        n[1] = char(0x93);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x56);
        r = c.length(m, n, n+1, 2);
        assert(r == 1);
    }
    {
        typedef std::codecvt_utf8_utf16<char32_t, 0x1000> C;
        C c;
        char n[4] = {char(0xF1), char(0x80), char(0x80), char(0x83)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[0] = char(0xE1);
        n[1] = char(0x80);
        n[2] = char(0x85);
        r = c.length(m, n, n+3, 2);
        assert(r == 0);

        n[0] = char(0xD1);
        n[1] = char(0x93);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x56);
        r = c.length(m, n, n+1, 2);
        assert(r == 1);
    }
    {
        typedef std::codecvt_utf8_utf16<char32_t, 0x10ffff, std::consume_header> C;
        C c;
        char n[7] = {char(0xEF), char(0xBB), char(0xBF), char(0xF1), char(0x80), char(0x80), char(0x83)};
        std::mbstate_t m;
        int r = c.length(m, n, n+7, 2);
        assert(r == 7);

        n[0] = char(0xE1);
        n[1] = char(0x80);
        n[2] = char(0x85);
        r = c.length(m, n, n+3, 2);
        assert(r == 3);

        n[0] = char(0xD1);
        n[1] = char(0x93);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x56);
        r = c.length(m, n, n+1, 2);
        assert(r == 1);
    }
    {
        typedef std::codecvt_utf8_utf16<char16_t> C;
        C c;
        char n[4] = {char(0xF1), char(0x80), char(0x80), char(0x83)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 4);

        n[0] = char(0xE1);
        n[1] = char(0x80);
        n[2] = char(0x85);
        r = c.length(m, n, n+3, 2);
        assert(r == 3);

        n[0] = char(0xD1);
        n[1] = char(0x93);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x56);
        r = c.length(m, n, n+1, 2);
        assert(r == 1);
    }
    {
        typedef std::codecvt_utf8_utf16<char16_t, 0x1000> C;
        C c;
        char n[4] = {char(0xF1), char(0x80), char(0x80), char(0x83)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[0] = char(0xE1);
        n[1] = char(0x80);
        n[2] = char(0x85);
        r = c.length(m, n, n+3, 2);
        assert(r == 0);

        n[0] = char(0xD1);
        n[1] = char(0x93);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x56);
        r = c.length(m, n, n+1, 2);
        assert(r == 1);
    }
    {
        typedef std::codecvt_utf8_utf16<char16_t, 0x10ffff, std::consume_header> C;
        C c;
        char n[7] = {char(0xEF), char(0xBB), char(0xBF), char(0xF1), char(0x80), char(0x80), char(0x83)};
        std::mbstate_t m;
        int r = c.length(m, n, n+7, 2);
        assert(r == 7);

        n[0] = char(0xE1);
        n[1] = char(0x80);
        n[2] = char(0x85);
        r = c.length(m, n, n+3, 2);
        assert(r == 3);

        n[0] = char(0xD1);
        n[1] = char(0x93);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x56);
        r = c.length(m, n, n+1, 2);
        assert(r == 1);
    }
}
