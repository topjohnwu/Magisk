//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <> class codecvt<char32_t, char, mbstate_t>
// template <> class codecvt<char16_t, char, mbstate_t>
// template <> class codecvt<char32_t, char16_t, mbstate_t>  // extension

// sanity check

#include <locale>
#include <codecvt>
#include <cassert>

#include <stdio.h>

int main()
{
    typedef std::codecvt<char32_t, char, std::mbstate_t> F32_8;
    typedef std::codecvt<char16_t, char, std::mbstate_t> F16_8;
    typedef std::codecvt_utf16<char32_t> F32_16;
    std::locale l = std::locale(std::locale::classic(), new F32_16);
    const F32_8& f32_8 = std::use_facet<F32_8>(std::locale::classic());
    const F32_16& f32_16 = std::use_facet<F32_16>(l);
    const F16_8& f16_8 = std::use_facet<F16_8>(std::locale::classic());
    std::mbstate_t mbs = {};
    F32_8::intern_type* c32p;
    F16_8::intern_type* c16p;
    F32_8::extern_type* c8p;
    const F32_8::intern_type* c_c32p;
    const F16_8::intern_type* c_c16p;
    const F32_8::extern_type* c_c8p;
    F32_8::intern_type c32;
    F16_8::intern_type c16[2];
    char c16c[4];
    char* c16cp;
    F32_8::extern_type c8[4];
    for (F32_8::intern_type c32x = 0; c32x < 0x110003; ++c32x)
    {
        if ((0xD800 <= c32x && c32x < 0xE000) || c32x >= 0x110000)
        {
            assert(f32_16.out(mbs, &c32x, &c32x+1, c_c32p, c16c+0, c16c+4, c16cp) == F32_8::error);
            assert(f32_8.out(mbs, &c32x, &c32x+1, c_c32p, c8, c8+4, c8p) == F32_8::error);
        }
        else
        {
            assert(f32_16.out(mbs, &c32x, &c32x+1, c_c32p, c16c, c16c+4, c16cp) == F32_8::ok);
            assert(c_c32p-&c32x == 1);
            if (c32x < 0x10000)
                assert(c16cp-c16c == 2);
            else
                assert(c16cp-c16c == 4);
            for (int i = 0; i < (c16cp - c16c) / 2; ++i)
                c16[i] = (unsigned char)c16c[2*i] << 8 | (unsigned char)c16c[2*i+1];
            c_c16p = c16 + (c16cp - c16c) / 2;
            assert(f16_8.out(mbs, c16, c_c16p, c_c16p, c8, c8+4, c8p) == F32_8::ok);
            if (c32x < 0x10000)
                assert(c_c16p-c16 == 1);
            else
                assert(c_c16p-c16 == 2);
            if (c32x < 0x80)
                assert(c8p-c8 == 1);
            else if (c32x < 0x800)
                assert(c8p-c8 == 2);
            else if (c32x < 0x10000)
                assert(c8p-c8 == 3);
            else
                assert(c8p-c8 == 4);
            c_c8p = c8p;
            assert(f32_8.in(mbs, c8, c_c8p, c_c8p, &c32, &c32+1, c32p) == F32_8::ok);
            if (c32x < 0x80)
                assert(c_c8p-c8 == 1);
            else if (c32x < 0x800)
                assert(c_c8p-c8 == 2);
            else if (c32x < 0x10000)
                assert(c_c8p-c8 == 3);
            else
                assert(c_c8p-c8 == 4);
            assert(c32p-&c32 == 1);
            assert(c32 == c32x);
            assert(f32_8.out(mbs, &c32x, &c32x+1, c_c32p, c8, c8+4, c8p) == F32_8::ok);
            assert(c_c32p-&c32x == 1);
            if (c32x < 0x80)
                assert(c8p-c8 == 1);
            else if (c32x < 0x800)
                assert(c8p-c8 == 2);
            else if (c32x < 0x10000)
                assert(c8p-c8 == 3);
            else
                assert(c8p-c8 == 4);
            c_c8p = c8p;
            assert(f16_8.in(mbs, c8, c_c8p, c_c8p, c16, c16+2, c16p) == F32_8::ok);
            if (c32x < 0x80)
                assert(c_c8p-c8 == 1);
            else if (c32x < 0x800)
                assert(c_c8p-c8 == 2);
            else if (c32x < 0x10000)
                assert(c_c8p-c8 == 3);
            else
                assert(c_c8p-c8 == 4);
            if (c32x < 0x10000)
                assert(c16p-c16 == 1);
            else
                assert(c16p-c16 == 2);
            for (int i = 0; i < c16p-c16; ++i)
            {
                c16c[2*i] = static_cast<char>(c16[i] >> 8);
                c16c[2*i+1] = static_cast<char>(c16[i]);
            }
            const char* c_c16cp = c16c + (c16p-c16)*2;
            assert(f32_16.in(mbs, c16c, c_c16cp, c_c16cp, &c32, &c32+1, c32p) == F32_8::ok);
            if (c32x < 0x10000)
                assert(c_c16cp-c16c == 2);
            else
                assert(c_c16cp-c16c == 4);
            assert(c32p-&c32 == 1);
            assert(c32 == c32x);
        }
    }
}
