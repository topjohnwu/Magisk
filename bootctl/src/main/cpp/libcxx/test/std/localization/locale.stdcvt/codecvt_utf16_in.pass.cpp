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

// result
//     in(stateT& state,
//        const externT* from, const externT* from_end, const externT*& from_next,
//        internT* to, internT* to_end, internT*& to_next) const;

#include <codecvt>
#include <cassert>

int main()
{
    {
        typedef std::codecvt_utf16<char32_t> C;
        C c;
        char32_t w = 0;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+4);
        assert(w == 0x40003);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x1000> C;
        C c;
        char32_t w = 0;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x10ffff, std::consume_header> C;
        C c;
        char32_t w = 0;
        char n[6] = {char(0xFE), char(0xFF), char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+6, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+6);
        assert(w == 0x40003);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x10ffff, std::little_endian> C;
        C c;
        char32_t w = 0;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+4);
        assert(w == 0x40003);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x1000, std::little_endian> C;
        C c;
        char32_t w = 0;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x10ffff,
                                   std::codecvt_mode(std::consume_header | std::little_endian)> C;

        C c;
        char32_t w = 0;
        char n[6] = {char(0xFF), char(0xFE), char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+6, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+6);
        assert(w == 0x40003);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t> C;
        C c;
        char32_t w = 0;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+4);
        assert(w == 0x40003);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x1000> C;
        C c;
        char32_t w = 0;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x10ffff, std::consume_header> C;
        C c;
        char32_t w = 0;
        char n[6] = {char(0xFE), char(0xFF), char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+6, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+6);
        assert(w == 0x40003);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x10ffff, std::little_endian> C;
        C c;
        char32_t w = 0;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+4);
        assert(w == 0x40003);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x1000, std::little_endian> C;
        C c;
        char32_t w = 0;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char32_t, 0x10ffff, std::codecvt_mode(
                                                         std::consume_header |
                                                         std::little_endian)> C;
        C c;
        char32_t w = 0;
        char n[6] = {char(0xFF), char(0xFE), char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        char32_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+6, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+6);
        assert(w == 0x40003);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }

    {
        typedef std::codecvt_utf16<char16_t> C;
        C c;
        char16_t w = 0;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        char16_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char16_t, 0x1000> C;
        C c;
        char16_t w = 0;
        char n[4] = {char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        char16_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char16_t, 0x10ffff, std::consume_header> C;
        C c;
        char16_t w = 0;
        char n[6] = {char(0xFE), char(0xFF), char(0xD8), char(0xC0), char(0xDC), char(0x03)};
        char16_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+6, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n+2);
        assert(w == 0);

        n[0] = char(0x10);
        n[1] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[0] = char(0x04);
        n[1] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[0] = char(0x00);
        n[1] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char16_t, 0x10ffff, std::little_endian> C;
        C c;
        char16_t w = 0;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        char16_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char16_t, 0x1000, std::little_endian> C;
        C c;
        char16_t w = 0;
        char n[4] = {char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        char16_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+4, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n);
        assert(w == 0);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
    {
        typedef std::codecvt_utf16<char16_t, 0x10ffff, std::codecvt_mode(
                                                         std::consume_header |
                                                         std::little_endian)> C;
        C c;
        char16_t w = 0;
        char n[6] = {char(0xFF), char(0xFE), char(0xC0), char(0xD8), char(0x03), char(0xDC)};
        char16_t* wp = nullptr;
        std::mbstate_t m;
        const char* np = nullptr;
        std::codecvt_base::result r = c.in(m, n, n+6, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::error);
        assert(wp == &w);
        assert(np == n+2);
        assert(w == 0);

        n[1] = char(0x10);
        n[0] = char(0x05);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x1005);

        n[1] = char(0x04);
        n[0] = char(0x53);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x453);

        w = 0x56;
        n[1] = char(0x00);
        n[0] = char(0x56);
        r = c.in(m, n, n+2, np, &w, &w+1, wp);
        assert(r == std::codecvt_base::ok);
        assert(wp == &w+1);
        assert(np == n+2);
        assert(w == 0x56);
    }
}
