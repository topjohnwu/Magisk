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

// result
// out(stateT& state,
//     const internT* from, const internT* from_end, const internT*& from_next,
//     externT* to, externT* to_end, externT*& to_next) const;

#include <codecvt>
#include <cassert>

template <class CharT, size_t = sizeof(CharT)>
struct TestHelper;
template <class CharT>
struct TestHelper<CharT, 2> {
  static void test();
};
template <class CharT>
struct TestHelper<CharT, 4> {
  static void test();
};

template <class CharT>
void TestHelper<CharT, 2>::test() {
  {
    typedef std::codecvt_utf8_utf16<CharT> C;
    C c;
    CharT w[2] = {0xD8C0, 0xDC03};
    char n[4] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, w, w + 2, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 2);
    assert(np == n + 4);
    assert(n[0] == char(0xF1));
    assert(n[1] == char(0x80));
    assert(n[2] == char(0x80));
    assert(n[3] == char(0x83));

    w[0] = 0x1005;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 3);
    assert(n[0] == char(0xE1));
    assert(n[1] == char(0x80));
    assert(n[2] == char(0x85));

    w[0] = 0x453;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 2);
    assert(n[0] == char(0xD1));
    assert(n[1] == char(0x93));

    w[0] = 0x56;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 1);
    assert(n[0] == char(0x56));
  }
  {
    typedef std::codecvt_utf8_utf16<CharT, 0x1000> C;
    C c;
    CharT w[2] = {0xD8C0, 0xDC03};
    char n[4] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, w, w + 2, wp, n, n + 4, np);
    assert(r == std::codecvt_base::error);
    assert(wp == w);
    assert(np == n);

    w[0] = 0x1005;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::error);
    assert(wp == w);
    assert(np == n);

    w[0] = 0x453;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 2);
    assert(n[0] == char(0xD1));
    assert(n[1] == char(0x93));

    w[0] = 0x56;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 1);
    assert(n[0] == char(0x56));
  }
  {
    typedef std::codecvt_utf8_utf16<CharT, 0x10ffff, std::generate_header> C;
    C c;
    CharT w[2] = {0xD8C0, 0xDC03};
    char n[7] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, w, w + 2, wp, n, n + 7, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 2);
    assert(np == n + 7);
    assert(n[0] == char(0xEF));
    assert(n[1] == char(0xBB));
    assert(n[2] == char(0xBF));
    assert(n[3] == char(0xF1));
    assert(n[4] == char(0x80));
    assert(n[5] == char(0x80));
    assert(n[6] == char(0x83));

    w[0] = 0x1005;
    r = c.out(m, w, w + 1, wp, n, n + 7, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 6);
    assert(n[0] == char(0xEF));
    assert(n[1] == char(0xBB));
    assert(n[2] == char(0xBF));
    assert(n[3] == char(0xE1));
    assert(n[4] == char(0x80));
    assert(n[5] == char(0x85));

    w[0] = 0x453;
    r = c.out(m, w, w + 1, wp, n, n + 7, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 5);
    assert(n[0] == char(0xEF));
    assert(n[1] == char(0xBB));
    assert(n[2] == char(0xBF));
    assert(n[3] == char(0xD1));
    assert(n[4] == char(0x93));

    w[0] = 0x56;
    r = c.out(m, w, w + 1, wp, n, n + 7, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 4);
    assert(n[0] == char(0xEF));
    assert(n[1] == char(0xBB));
    assert(n[2] == char(0xBF));
    assert(n[3] == char(0x56));
  }
}

template <class CharT>
void TestHelper<CharT, 4>::test() {
  {
    typedef std::codecvt_utf8_utf16<CharT> C;
    C c;
    CharT w[2] = {0xD8C0, 0xDC03};
    char n[4] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, w, w + 2, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 2);
    assert(np == n + 4);
    assert(n[0] == char(0xF1));
    assert(n[1] == char(0x80));
    assert(n[2] == char(0x80));
    assert(n[3] == char(0x83));

    w[0] = 0x1005;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 3);
    assert(n[0] == char(0xE1));
    assert(n[1] == char(0x80));
    assert(n[2] == char(0x85));

    w[0] = 0x453;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 2);
    assert(n[0] == char(0xD1));
    assert(n[1] == char(0x93));

    w[0] = 0x56;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 1);
    assert(n[0] == char(0x56));
  }
  {
    typedef std::codecvt_utf8_utf16<CharT, 0x1000> C;
    C c;
    CharT w[2] = {0xD8C0, 0xDC03};
    char n[4] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, w, w + 2, wp, n, n + 4, np);
    assert(r == std::codecvt_base::error);
    assert(wp == w);
    assert(np == n);

    w[0] = 0x1005;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::error);
    assert(wp == w);
    assert(np == n);

    w[0] = 0x453;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 2);
    assert(n[0] == char(0xD1));
    assert(n[1] == char(0x93));

    w[0] = 0x56;
    r = c.out(m, w, w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 1);
    assert(n[0] == char(0x56));
  }
  {
    typedef std::codecvt_utf8_utf16<CharT, 0x10ffff, std::generate_header> C;
    C c;
    CharT w[2] = {0xD8C0, 0xDC03};
    char n[7] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, w, w + 2, wp, n, n + 7, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 2);
    assert(np == n + 7);
    assert(n[0] == char(0xEF));
    assert(n[1] == char(0xBB));
    assert(n[2] == char(0xBF));
    assert(n[3] == char(0xF1));
    assert(n[4] == char(0x80));
    assert(n[5] == char(0x80));
    assert(n[6] == char(0x83));

    w[0] = 0x1005;
    r = c.out(m, w, w + 1, wp, n, n + 7, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 6);
    assert(n[0] == char(0xEF));
    assert(n[1] == char(0xBB));
    assert(n[2] == char(0xBF));
    assert(n[3] == char(0xE1));
    assert(n[4] == char(0x80));
    assert(n[5] == char(0x85));

    w[0] = 0x453;
    r = c.out(m, w, w + 1, wp, n, n + 7, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 5);
    assert(n[0] == char(0xEF));
    assert(n[1] == char(0xBB));
    assert(n[2] == char(0xBF));
    assert(n[3] == char(0xD1));
    assert(n[4] == char(0x93));

    w[0] = 0x56;
    r = c.out(m, w, w + 1, wp, n, n + 7, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == w + 1);
    assert(np == n + 4);
    assert(n[0] == char(0xEF));
    assert(n[1] == char(0xBB));
    assert(n[2] == char(0xBF));
    assert(n[3] == char(0x56));
  }
}

int main() {
#ifndef _WIN32
  TestHelper<wchar_t>::test();
#endif
  TestHelper<char32_t>::test();
  TestHelper<char16_t>::test();
}
