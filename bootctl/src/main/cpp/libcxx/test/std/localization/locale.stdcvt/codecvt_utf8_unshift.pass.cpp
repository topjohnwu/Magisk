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
// class codecvt_utf8
//     : public codecvt<Elem, char, mbstate_t>
// {
//     // unspecified
// };

// result
//     unshift(stateT& state,
//             externT* to, externT* to_end, externT*& to_next) const;

#include <codecvt>
#include <cassert>

int main()
{
    {
        typedef std::codecvt_utf8<wchar_t> C;
        C c;
        char n[4] = {0};
        std::mbstate_t m;
        char* np = nullptr;
        std::codecvt_base::result r = c.unshift(m, n, n+4, np);
        assert(r == std::codecvt_base::noconv);
    }
    {
        typedef std::codecvt_utf8<char16_t> C;
        C c;
        char n[4] = {0};
        std::mbstate_t m;
        char* np = nullptr;
        std::codecvt_base::result r = c.unshift(m, n, n+4, np);
        assert(r == std::codecvt_base::noconv);
    }
    {
        typedef std::codecvt_utf8<char32_t> C;
        C c;
        char n[4] = {0};
        std::mbstate_t m;
        char* np = nullptr;
        std::codecvt_base::result r = c.unshift(m, n, n+4, np);
        assert(r == std::codecvt_base::noconv);
    }
}
