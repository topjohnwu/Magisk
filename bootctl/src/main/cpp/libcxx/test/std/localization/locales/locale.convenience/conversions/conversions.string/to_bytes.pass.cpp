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

// byte_string to_bytes(Elem wchar);
// byte_string to_bytes(const Elem* wptr);
// byte_string to_bytes(const wide_string& wstr);
// byte_string to_bytes(const Elem* first, const Elem* last);

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
    std::wstring_convert<std::codecvt_utf8<CharT> > myconv;
    std::wstring ws(1, CharT(0x1005));
    std::string bs = myconv.to_bytes(ws[0]);
    assert(bs == "\xE1\x80\x85\x00");
    bs = myconv.to_bytes(ws.c_str());
    assert(bs == "\xE1\x80\x85\x00");
    bs = myconv.to_bytes(ws);
    assert(bs == "\xE1\x80\x85\x00");
    bs = myconv.to_bytes(ws.data(), ws.data() + ws.size());
    assert(bs == "\xE1\x80\x85\x00");
    bs = myconv.to_bytes(L"");
    assert(bs.size() == 0);
  }
}

template <class CharT>
void TestHelper<CharT, 4>::test() {
  static_assert((std::is_same<CharT, wchar_t>::value), "");
  {
    std::wstring_convert<std::codecvt_utf8<CharT> > myconv;
    std::wstring ws(1, CharT(0x40003));
    std::string bs = myconv.to_bytes(ws[0]);
    assert(bs == "\xF1\x80\x80\x83");
    bs = myconv.to_bytes(ws.c_str());
    assert(bs == "\xF1\x80\x80\x83");
    bs = myconv.to_bytes(ws);
    assert(bs == "\xF1\x80\x80\x83");
    bs = myconv.to_bytes(ws.data(), ws.data() + ws.size());
    assert(bs == "\xF1\x80\x80\x83");
    bs = myconv.to_bytes(L"");
    assert(bs.size() == 0);
  }
}

int main() { TestHelper<wchar_t>::test(); }
