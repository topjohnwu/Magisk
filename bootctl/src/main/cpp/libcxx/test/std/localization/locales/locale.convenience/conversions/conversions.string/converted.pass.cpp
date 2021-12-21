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

// size_t converted() const;

#include <locale>
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
  static_assert((std::is_same<CharT, wchar_t>::value), "");
  {
    typedef std::codecvt_utf8<CharT> Codecvt;
    typedef std::wstring_convert<Codecvt> Myconv;
    Myconv myconv;
    assert(myconv.converted() == 0);
    std::string bs = myconv.to_bytes(L"\x1005");
    assert(myconv.converted() == 1);
    bs = myconv.to_bytes(L"\x1005\x65");
    assert(myconv.converted() == 2);
    std::wstring ws = myconv.from_bytes("\xE1\x80\x85");
    assert(myconv.converted() == 3);
  }
}

template <class CharT>
void TestHelper<CharT, 4>::test() {
  static_assert((std::is_same<CharT, wchar_t>::value), "");
  {
    typedef std::codecvt_utf8<CharT> Codecvt;
    typedef std::wstring_convert<Codecvt> Myconv;
    Myconv myconv;
    assert(myconv.converted() == 0);
    std::string bs = myconv.to_bytes(L"\x40003");
    assert(myconv.converted() == 1);
    bs = myconv.to_bytes(L"\x40003\x65");
    assert(myconv.converted() == 2);
    std::wstring ws = myconv.from_bytes("\xF1\x80\x80\x83");
    assert(myconv.converted() == 4);
  }
}

int main() { TestHelper<wchar_t>::test(); }
