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
// class codecvt_utf16
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
        typedef std::codecvt_utf16<wchar_t> C;
        C c;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 4);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<wchar_t, 0x1000> C;
        C c;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 0);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header> C;
        C c;
        char n[6] = {char(0xFE), char(0xFF), char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        std::mbstate_t m;
        int r = c.length(m, n, n+6, 2);
        assert(r == 6);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian> C;
        C c;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 4);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<wchar_t, 0x1000, std::little_endian> C;
        C c;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 0);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<wchar_t, 0x10ffff, std::codecvt_mode(
                                                         std::consume_header |
                                                         std::little_endian)> C;
        C c;
        char n[6] = {char(0xFF), char(0xFE), char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        std::mbstate_t m;
        int r = c.length(m, n, n+6, 2);
        assert(r == 6);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char32_t> C;
        C c;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 4);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x1000> C;
        C c;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 0);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x10ffff, std::consume_header> C;
        C c;
        char n[6] = {char(0xFE), char(0xFF), char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        std::mbstate_t m;
        int r = c.length(m, n, n+6, 2);
        assert(r == 6);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x10ffff, std::little_endian> C;
        C c;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 4);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x1000, std::little_endian> C;
        C c;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 0);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x10ffff, std::codecvt_mode(
                                                         std::consume_header |
                                                         std::little_endian)> C;
        C c;
        char n[6] = {char(0xFF), char(0xFE), char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        std::mbstate_t m;
        int r = c.length(m, n, n+6, 2);
        assert(r == 6);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }

    {
        typedef std::codecvt_utf16<char16_t> C;
        C c;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char16_t, 0x1000> C;
        C c;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 0);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char16_t, 0x10ffff, std::consume_header> C;
        C c;
        char n[6] = {char(0xFE), char(0xFF), char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        std::mbstate_t m;
        int r = c.length(m, n, n+6, 2);
        assert(r == 2);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char16_t, 0x10ffff, std::little_endian> C;
        C c;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char16_t, 0x1000, std::little_endian> C;
        C c;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        std::mbstate_t m;
        int r = c.length(m, n, n+4, 2);
        assert(r == 0);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 0);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
    {
        typedef std::codecvt_utf16<char16_t, 0x10ffff, std::codecvt_mode(
                                                         std::consume_header |
                                                         std::little_endian)> C;
        C c;
        char n[6] = {char(0xFF), char(0xFE), char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        std::mbstate_t m;
        int r = c.length(m, n, n+6, 2);
        assert(r == 2);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);

        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.length(m, n, n+2, 2);
        assert(r == 2);
    }
}
