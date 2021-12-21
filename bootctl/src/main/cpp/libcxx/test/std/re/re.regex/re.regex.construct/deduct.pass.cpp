//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-deduction-guides


// template<class ForwardIterator>
// basic_regex(ForwardIterator, ForwardIterator,
//             regex_constants::syntax_option_type = regex_constants::ECMAScript)
// -> basic_regex<typename iterator_traits<ForwardIterator>::value_type>;


#include <regex>
#include <string>
#include <iterator>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_iterators.h"
#include "test_allocator.h"

using namespace std::literals;

struct A {};

int main()
{

//  Test the explicit deduction guides
    {
//  basic_regex(ForwardIterator, ForwardIterator)
    std::string s1("\\(a\\)");
    std::basic_regex re(s1.begin(), s1.end());

    static_assert(std::is_same_v<decltype(re), std::basic_regex<char>>, "");
    assert(re.flags() == std::regex_constants::ECMAScript);
    assert(re.mark_count() == 0);
    }

    {
    std::wstring s1(L"\\(a\\)");
    std::basic_regex re(s1.begin(), s1.end(), std::regex_constants::basic);

    static_assert(std::is_same_v<decltype(re), std::basic_regex<wchar_t>>, "");
    assert(re.flags() == std::regex_constants::basic);
    assert(re.mark_count() == 1);
    }

//  Test the implicit deduction guides
    {
//  basic_regex(string);
    std::basic_regex re("(a([bc]))"s);
    static_assert(std::is_same_v<decltype(re), std::basic_regex<char>>, "");
    assert(re.flags() == std::regex_constants::ECMAScript);
    assert(re.mark_count() == 2);
    }

    {
//  basic_regex(string, flag_type);
    std::basic_regex re(L"(a([bc]))"s, std::regex_constants::awk);
    static_assert(std::is_same_v<decltype(re), std::basic_regex<wchar_t>>, "");
    assert(re.flags() == std::regex_constants::awk);
    assert(re.mark_count() == 2);
    }

    {
//  basic_regex(const charT*);
    std::basic_regex re("ABCDE");
    static_assert(std::is_same_v<decltype(re), std::basic_regex<char>>, "");
    assert(re.flags() == std::regex_constants::ECMAScript);
    assert(re.mark_count() == 0);
    }

    {
//  basic_regex(const charT*, flag_type);
    std::basic_regex re(L"ABCDE", std::regex_constants::grep);
    static_assert(std::is_same_v<decltype(re), std::basic_regex<wchar_t>>, "");
    assert(re.flags() == std::regex_constants::grep);
    assert(re.mark_count() == 0);
    }

    {
//  basic_regex(const charT*, size_t);
    std::basic_regex re("ABCDEDEF", 7);
    static_assert(std::is_same_v<decltype(re), std::basic_regex<char>>, "");
    assert(re.flags() == std::regex_constants::ECMAScript);
    assert(re.mark_count() == 0);
    }

    {
//  basic_regex(const charT*, size_t, flag_type);
    std::basic_regex re(L"ABCDEDEF", 8, std::regex_constants::awk);
    static_assert(std::is_same_v<decltype(re), std::basic_regex<wchar_t>>, "");
    assert(re.flags() == std::regex_constants::awk);
    assert(re.mark_count() == 0);
    }

    {
//  basic_regex(const basic_regex &);
    std::basic_regex<char> source;
    std::basic_regex re(source);
    static_assert(std::is_same_v<decltype(re), std::basic_regex<char>>, "");
    assert(re.flags() == source.flags());
    assert(re.mark_count() == source.mark_count());
    }

    {
//  template<class ST, class SA>
//         explicit basic_regex(const basic_string<charT, ST, SA>& p,
//                              flag_type f = regex_constants::ECMAScript);
    }

    {
//  basic_regex(initializer_list);
    std::basic_regex re({'A', 'B', 'F', 'E', 'D'});
    static_assert(std::is_same_v<decltype(re), std::basic_regex<char>>, "");
    assert(re.flags() == std::regex_constants::ECMAScript);
    assert(re.mark_count() == 0);
    }

    {
//  basic_regex(initializer_list, flag_type);
    std::basic_regex re({L'A', L'B', L'F', L'E', L'D'}, std::regex_constants::grep);
    static_assert(std::is_same_v<decltype(re), std::basic_regex<wchar_t>>, "");
    assert(re.flags() == std::regex_constants::grep);
    assert(re.mark_count() == 0);
    }
}
