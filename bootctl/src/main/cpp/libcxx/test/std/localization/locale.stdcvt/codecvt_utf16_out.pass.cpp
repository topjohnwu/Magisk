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
  // Nothing to do, the conversion in unsupported
}

template <class CharT>
void TestHelper<CharT, 4>::test() {
  {
    typedef std::codecvt_utf16<CharT> C;
    C c;
    CharT w = 0x40003;
    char n[4] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 4);
    assert(n[0] == char(0xD8));
    assert(n[1] == char(0xC0));
    assert(n[2] == char(0xDC));
    assert(n[3] == char(0x03));

    w = 0x1005;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 2);
    assert(n[0] == char(0x10));
    assert(n[1] == char(0x05));
    assert(n[2] == char(0xDC));
    assert(n[3] == char(0x03));

    w = 0x453;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 2);
    assert(n[0] == char(0x04));
    assert(n[1] == char(0x53));
    assert(n[2] == char(0xDC));
    assert(n[3] == char(0x03));

    w = 0x56;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 2);
    assert(n[0] == char(0x00));
    assert(n[1] == char(0x56));
    assert(n[2] == char(0xDC));
    assert(n[3] == char(0x03));
  }
  {
    typedef std::codecvt_utf16<CharT, 0x1000> C;
    C c;
    CharT w = 0x40003;
    char n[4] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::error);
    assert(wp == &w);
    assert(np == n);
    assert(n[0] == char(0));
    assert(n[1] == char(0));
    assert(n[2] == char(0));
    assert(n[3] == char(0));

    w = 0x1005;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::error);
    assert(wp == &w);
    assert(np == n);
    assert(n[0] == char(0));
    assert(n[1] == char(0));
    assert(n[2] == char(0));
    assert(n[3] == char(0));

    w = 0x453;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 2);
    assert(n[0] == char(0x04));
    assert(n[1] == char(0x53));
    assert(n[2] == char(0));
    assert(n[3] == char(0));

    w = 0x56;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 2);
    assert(n[0] == char(0x00));
    assert(n[1] == char(0x56));
    assert(n[2] == char(0));
    assert(n[3] == char(0));
  }
  {
    typedef std::codecvt_utf16<CharT, 0x10ffff, std::generate_header> C;
    C c;
    CharT w = 0x40003;
    char n[6] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, &w, &w + 1, wp, n, n + 6, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 6);
    assert(n[0] == char(0xFE));
    assert(n[1] == char(0xFF));
    assert(n[2] == char(0xD8));
    assert(n[3] == char(0xC0));
    assert(n[4] == char(0xDC));
    assert(n[5] == char(0x03));

    w = 0x1005;
    r = c.out(m, &w, &w + 1, wp, n, n + 6, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 4);
    assert(n[0] == char(0xFE));
    assert(n[1] == char(0xFF));
    assert(n[2] == char(0x10));
    assert(n[3] == char(0x05));
    assert(n[4] == char(0xDC));
    assert(n[5] == char(0x03));

    w = 0x453;
    r = c.out(m, &w, &w + 1, wp, n, n + 6, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 4);
    assert(n[0] == char(0xFE));
    assert(n[1] == char(0xFF));
    assert(n[2] == char(0x04));
    assert(n[3] == char(0x53));
    assert(n[4] == char(0xDC));
    assert(n[5] == char(0x03));

    w = 0x56;
    r = c.out(m, &w, &w + 1, wp, n, n + 6, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 4);
    assert(n[0] == char(0xFE));
    assert(n[1] == char(0xFF));
    assert(n[2] == char(0x00));
    assert(n[3] == char(0x56));
    assert(n[4] == char(0xDC));
    assert(n[5] == char(0x03));
  }

  {
    typedef std::codecvt_utf16<CharT, 0x10FFFF, std::little_endian> C;
    C c;
    CharT w = 0x40003;
    char n[4] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 4);
    assert(n[1] == char(0xD8));
    assert(n[0] == char(0xC0));
    assert(n[3] == char(0xDC));
    assert(n[2] == char(0x03));

    w = 0x1005;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 2);
    assert(n[1] == char(0x10));
    assert(n[0] == char(0x05));
    assert(n[3] == char(0xDC));
    assert(n[2] == char(0x03));

    w = 0x453;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 2);
    assert(n[1] == char(0x04));
    assert(n[0] == char(0x53));
    assert(n[3] == char(0xDC));
    assert(n[2] == char(0x03));

    w = 0x56;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 2);
    assert(n[1] == char(0x00));
    assert(n[0] == char(0x56));
    assert(n[3] == char(0xDC));
    assert(n[2] == char(0x03));
  }
  {
    typedef std::codecvt_utf16<CharT, 0x1000, std::little_endian> C;
    C c;
    CharT w = 0x40003;
    char n[4] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::error);
    assert(wp == &w);
    assert(np == n);
    assert(n[1] == char(0));
    assert(n[0] == char(0));
    assert(n[3] == char(0));
    assert(n[2] == char(0));

    w = 0x1005;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::error);
    assert(wp == &w);
    assert(np == n);
    assert(n[1] == char(0));
    assert(n[0] == char(0));
    assert(n[3] == char(0));
    assert(n[2] == char(0));

    w = 0x453;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 2);
    assert(n[1] == char(0x04));
    assert(n[0] == char(0x53));
    assert(n[3] == char(0));
    assert(n[2] == char(0));

    w = 0x56;
    r = c.out(m, &w, &w + 1, wp, n, n + 4, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 2);
    assert(n[1] == char(0x00));
    assert(n[0] == char(0x56));
    assert(n[3] == char(0));
    assert(n[2] == char(0));
  }
  {
    typedef std::codecvt_utf16<CharT, 0x10ffff,
                               std::codecvt_mode(std::generate_header |
                                                 std::little_endian)>
        C;
    C c;
    CharT w = 0x40003;
    char n[6] = {0};
    const CharT* wp = nullptr;
    std::mbstate_t m;
    char* np = nullptr;
    std::codecvt_base::result r = c.out(m, &w, &w + 1, wp, n, n + 6, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 6);
    assert(n[1] == char(0xFE));
    assert(n[0] == char(0xFF));
    assert(n[3] == char(0xD8));
    assert(n[2] == char(0xC0));
    assert(n[5] == char(0xDC));
    assert(n[4] == char(0x03));

    w = 0x1005;
    r = c.out(m, &w, &w + 1, wp, n, n + 6, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 4);
    assert(n[1] == char(0xFE));
    assert(n[0] == char(0xFF));
    assert(n[3] == char(0x10));
    assert(n[2] == char(0x05));
    assert(n[5] == char(0xDC));
    assert(n[4] == char(0x03));

    w = 0x453;
    r = c.out(m, &w, &w + 1, wp, n, n + 6, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 4);
    assert(n[1] == char(0xFE));
    assert(n[0] == char(0xFF));
    assert(n[3] == char(0x04));
    assert(n[2] == char(0x53));
    assert(n[5] == char(0xDC));
    assert(n[4] == char(0x03));

    w = 0x56;
    r = c.out(m, &w, &w + 1, wp, n, n + 6, np);
    assert(r == std::codecvt_base::ok);
    assert(wp == &w + 1);
    assert(np == n + 4);
    assert(n[1] == char(0xFE));
    assert(n[0] == char(0xFF));
    assert(n[3] == char(0x00));
    assert(n[2] == char(0x56));
    assert(n[5] == char(0xDC));
    assert(n[4] == char(0x03));
  }
}

int main() {
  TestHelper<char32_t>::test();
  TestHelper<wchar_t>::test();
}
