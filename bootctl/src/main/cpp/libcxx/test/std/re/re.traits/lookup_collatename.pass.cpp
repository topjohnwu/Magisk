//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// NetBSD does not support LC_COLLATE at the moment
// XFAIL: netbsd

// REQUIRES: locale.cs_CZ.ISO8859-2

// <regex>

// template <class charT> struct regex_traits;

// template <class ForwardIterator>
//   string_type
//   lookup_collatename(ForwardIterator first, ForwardIterator last) const;

// TODO: investigation needed
// XFAIL: linux-gnu

#include <regex>
#include <iterator>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "platform_support.h" // locale name macros

template <class char_type>
void
test(const char_type* A, const std::basic_string<char_type>& expected)
{
    std::regex_traits<char_type> t;
    typedef forward_iterator<const char_type*> F;
    assert(t.lookup_collatename(F(A), F(A + t.length(A))) == expected);
}

int main()
{
    test("NUL", std::string("\x00", 1));
    test("alert", std::string("\x07"));
    test("backspace", std::string("\x08"));
    test("tab", std::string("\x09"));
    test("carriage-return", std::string("\x0D"));
    test("newline", std::string("\x0A"));
    test("vertical-tab", std::string("\x0B"));
    test("form-feed", std::string("\x0C"));
    test("space", std::string(" "));
    test("exclamation-mark", std::string("!"));
    test("quotation-mark", std::string("\""));
    test("number-sign", std::string("#"));
    test("dollar-sign", std::string("$"));
    test("percent-sign", std::string("%"));
    test("ampersand", std::string("&"));
    test("apostrophe", std::string("\'"));
    test("left-parenthesis", std::string("("));
    test("right-parenthesis", std::string(")"));
    test("asterisk", std::string("*"));
    test("plus-sign", std::string("+"));
    test("comma", std::string(","));
    test("hyphen-minus", std::string("-"));
    test("hyphen", std::string("-"));
    test("full-stop", std::string("."));
    test("period", std::string("."));
    test("slash", std::string("/"));
    test("solidus", std::string("/"));
    test("zero", std::string("0"));
    test("one", std::string("1"));
    test("two", std::string("2"));
    test("three", std::string("3"));
    test("four", std::string("4"));
    test("five", std::string("5"));
    test("six", std::string("6"));
    test("seven", std::string("7"));
    test("eight", std::string("8"));
    test("nine", std::string("9"));
    test("colon", std::string(":"));
    test("semicolon", std::string(";"));
    test("less-than-sign", std::string("<"));
    test("equals-sign", std::string("="));
    test("greater-than-sign", std::string(">"));
    test("question-mark", std::string("?"));
    test("commercial-at", std::string("@"));
    for (char c = 'A'; c <= 'Z'; ++c)
    {
        const char a[2] = {c};
        test(a, std::string(a));
    }
    test("left-square-bracket", std::string("["));
    test("backslash", std::string("\\"));
    test("reverse-solidus", std::string("\\"));
    test("right-square-bracket", std::string("]"));
    test("circumflex-accent", std::string("^"));
    test("circumflex", std::string("^"));
    test("low-line", std::string("_"));
    test("underscore", std::string("_"));
    test("grave-accent", std::string("`"));
    for (char c = 'a'; c <= 'z'; ++c)
    {
        const char a[2] = {c};
        test(a, std::string(a));
    }
    test("left-brace", std::string("{"));
    test("left-curly-bracket", std::string("{"));
    test("vertical-line", std::string("|"));
    test("right-brace", std::string("}"));
    test("right-curly-bracket", std::string("}"));
    test("tilde", std::string("~"));

    test("tild", std::string(""));
    test("ch", std::string(""));
    std::locale::global(std::locale(LOCALE_cs_CZ_ISO8859_2));
    test("ch", std::string("ch"));
    std::locale::global(std::locale("C"));

    test(L"NUL", std::wstring(L"\x00", 1));
    test(L"alert", std::wstring(L"\x07"));
    test(L"backspace", std::wstring(L"\x08"));
    test(L"tab", std::wstring(L"\x09"));
    test(L"carriage-return", std::wstring(L"\x0D"));
    test(L"newline", std::wstring(L"\x0A"));
    test(L"vertical-tab", std::wstring(L"\x0B"));
    test(L"form-feed", std::wstring(L"\x0C"));
    test(L"space", std::wstring(L" "));
    test(L"exclamation-mark", std::wstring(L"!"));
    test(L"quotation-mark", std::wstring(L"\""));
    test(L"number-sign", std::wstring(L"#"));
    test(L"dollar-sign", std::wstring(L"$"));
    test(L"percent-sign", std::wstring(L"%"));
    test(L"ampersand", std::wstring(L"&"));
    test(L"apostrophe", std::wstring(L"\'"));
    test(L"left-parenthesis", std::wstring(L"("));
    test(L"right-parenthesis", std::wstring(L")"));
    test(L"asterisk", std::wstring(L"*"));
    test(L"plus-sign", std::wstring(L"+"));
    test(L"comma", std::wstring(L","));
    test(L"hyphen-minus", std::wstring(L"-"));
    test(L"hyphen", std::wstring(L"-"));
    test(L"full-stop", std::wstring(L"."));
    test(L"period", std::wstring(L"."));
    test(L"slash", std::wstring(L"/"));
    test(L"solidus", std::wstring(L"/"));
    test(L"zero", std::wstring(L"0"));
    test(L"one", std::wstring(L"1"));
    test(L"two", std::wstring(L"2"));
    test(L"three", std::wstring(L"3"));
    test(L"four", std::wstring(L"4"));
    test(L"five", std::wstring(L"5"));
    test(L"six", std::wstring(L"6"));
    test(L"seven", std::wstring(L"7"));
    test(L"eight", std::wstring(L"8"));
    test(L"nine", std::wstring(L"9"));
    test(L"colon", std::wstring(L":"));
    test(L"semicolon", std::wstring(L";"));
    test(L"less-than-sign", std::wstring(L"<"));
    test(L"equals-sign", std::wstring(L"="));
    test(L"greater-than-sign", std::wstring(L">"));
    test(L"question-mark", std::wstring(L"?"));
    test(L"commercial-at", std::wstring(L"@"));
    for (wchar_t c = L'A'; c <= L'Z'; ++c)
    {
        const wchar_t a[2] = {c};
        test(a, std::wstring(a));
    }
    test(L"left-square-bracket", std::wstring(L"["));
    test(L"backslash", std::wstring(L"\\"));
    test(L"reverse-solidus", std::wstring(L"\\"));
    test(L"right-square-bracket", std::wstring(L"]"));
    test(L"circumflex-accent", std::wstring(L"^"));
    test(L"circumflex", std::wstring(L"^"));
    test(L"low-line", std::wstring(L"_"));
    test(L"underscore", std::wstring(L"_"));
    test(L"grave-accent", std::wstring(L"`"));
    for (wchar_t c = L'a'; c <= L'z'; ++c)
    {
        const wchar_t a[2] = {c};
        test(a, std::wstring(a));
    }
    test(L"left-brace", std::wstring(L"{"));
    test(L"left-curly-bracket", std::wstring(L"{"));
    test(L"vertical-line", std::wstring(L"|"));
    test(L"right-brace", std::wstring(L"}"));
    test(L"right-curly-bracket", std::wstring(L"}"));
    test(L"tilde", std::wstring(L"~"));

    test(L"tild", std::wstring(L""));
    test(L"ch", std::wstring(L""));
    std::locale::global(std::locale(LOCALE_cs_CZ_ISO8859_2));
    test(L"ch", std::wstring(L"ch"));
    std::locale::global(std::locale("C"));
}
