//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT, class traits = regex_traits<charT>>
// class basic_regex
// {
// public:
//     // constants:
//     static constexpr regex_constants::syntax_option_type icase = regex_constants::icase;
//     static constexpr regex_constants::syntax_option_type nosubs = regex_constants::nosubs;
//     static constexpr regex_constants::syntax_option_type optimize = regex_constants::optimize;
//     static constexpr regex_constants::syntax_option_type collate = regex_constants::collate;
//     static constexpr regex_constants::syntax_option_type ECMAScript = regex_constants::ECMAScript;
//     static constexpr regex_constants::syntax_option_type basic = regex_constants::basic;
//     static constexpr regex_constants::syntax_option_type extended = regex_constants::extended;
//     static constexpr regex_constants::syntax_option_type awk = regex_constants::awk;
//     static constexpr regex_constants::syntax_option_type grep = regex_constants::grep;
//     static constexpr regex_constants::syntax_option_type egrep = regex_constants::egrep;

#include <regex>
#include <type_traits>
#include "test_macros.h"

template <class T>
void where(const T &) {}

template <class CharT>
void
test()
{
    typedef std::basic_regex<CharT> BR;
    static_assert((BR::icase == std::regex_constants::icase), "");
    static_assert((BR::nosubs == std::regex_constants::nosubs), "");
    static_assert((BR::optimize == std::regex_constants::optimize), "");
    static_assert((BR::collate == std::regex_constants::collate), "");
    static_assert((BR::ECMAScript == std::regex_constants::ECMAScript), "");
    static_assert((BR::basic == std::regex_constants::basic), "");
    static_assert((BR::extended == std::regex_constants::extended), "");
    static_assert((BR::awk == std::regex_constants::awk), "");
    static_assert((BR::grep == std::regex_constants::grep), "");
    static_assert((BR::egrep == std::regex_constants::egrep), "");
    where(BR::icase);
    where(BR::nosubs);
    where(BR::optimize);
    where(BR::collate);
    where(BR::ECMAScript);
    where(BR::basic);
    where(BR::extended);
    where(BR::awk);
    where(BR::grep);
    where(BR::egrep);
}

int main()
{
    test<char>();
    test<wchar_t>();
}
