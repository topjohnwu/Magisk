//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// template <class ForwardIterator>
//    basic_regex(ForwardIterator first, ForwardIterator last,
//                flag_type f = regex_constants::ECMAScript);

#include <regex>
#include <cassert>

#include "test_iterators.h"
#include "test_macros.h"

template <class Iter>
void
test(Iter first, Iter last, std::regex_constants::syntax_option_type f, unsigned mc)
{
    std::basic_regex<typename std::iterator_traits<Iter>::value_type> r(first, last, f);
    assert(r.flags() == f);
    assert(r.mark_count() == mc);
}

int main()
{
    typedef forward_iterator<std::string::const_iterator> F;
    std::string s1("\\(a\\)");
    std::string s2("\\(a[bc]\\)");
    std::string s3("\\(a\\([bc]\\)\\)");
    std::string s4("(a([bc]))");

    test(F(s1.begin()), F(s1.end()), std::regex_constants::basic, 1);
    test(F(s2.begin()), F(s2.end()), std::regex_constants::basic, 1);
    test(F(s3.begin()), F(s3.end()), std::regex_constants::basic, 2);
    test(F(s4.begin()), F(s4.end()), std::regex_constants::basic, 0);

    test(F(s1.begin()), F(s1.end()), std::regex_constants::extended, 0);
    test(F(s2.begin()), F(s2.end()), std::regex_constants::extended, 0);
    test(F(s3.begin()), F(s3.end()), std::regex_constants::extended, 0);
    test(F(s4.begin()), F(s4.end()), std::regex_constants::extended, 2);

    test(F(s1.begin()), F(s1.end()), std::regex_constants::ECMAScript, 0);
    test(F(s2.begin()), F(s2.end()), std::regex_constants::ECMAScript, 0);
    test(F(s3.begin()), F(s3.end()), std::regex_constants::ECMAScript, 0);
    test(F(s4.begin()), F(s4.end()), std::regex_constants::ECMAScript, 2);

    test(F(s1.begin()), F(s1.end()), std::regex_constants::awk, 0);
    test(F(s2.begin()), F(s2.end()), std::regex_constants::awk, 0);
    test(F(s3.begin()), F(s3.end()), std::regex_constants::awk, 0);
    test(F(s4.begin()), F(s4.end()), std::regex_constants::awk, 2);

    test(F(s1.begin()), F(s1.end()), std::regex_constants::grep, 1);
    test(F(s2.begin()), F(s2.end()), std::regex_constants::grep, 1);
    test(F(s3.begin()), F(s3.end()), std::regex_constants::grep, 2);
    test(F(s4.begin()), F(s4.end()), std::regex_constants::grep, 0);

    test(F(s1.begin()), F(s1.end()), std::regex_constants::egrep, 0);
    test(F(s2.begin()), F(s2.end()), std::regex_constants::egrep, 0);
    test(F(s3.begin()), F(s3.end()), std::regex_constants::egrep, 0);
    test(F(s4.begin()), F(s4.end()), std::regex_constants::egrep, 2);
}
