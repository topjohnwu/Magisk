//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT> struct regex_traits;

// template <class ForwardIterator>
//   char_class_type
//   lookup_classname(ForwardIterator first, ForwardIterator last,
//                    bool icase = false) const;

#include <regex>
#include <cassert>
#include "test_macros.h"
#include "test_iterators.h"

template <class char_type>
void
test(const char_type* A,
     typename std::regex_traits<char_type>::char_class_type expected,
     bool icase = false)
{
    typedef typename std::regex_traits<char_type>::char_class_type char_class_type;
    std::regex_traits<char_type> t;
    typedef forward_iterator<const char_type*> F;
    char_class_type result = t.lookup_classname(F(A), F(A + t.length(A)), icase);
    assert(result == expected);
}

template <class char_type>
void
test_w(const char_type* A,
       typename std::regex_traits<char_type>::char_class_type expected,
        bool icase = false)
{
    typedef typename std::regex_traits<char_type>::char_class_type char_class_type;
    std::regex_traits<char_type> t;
    typedef forward_iterator<const char_type*> F;
    char_class_type result = t.lookup_classname(F(A), F(A + t.length(A)), icase);
    assert((result & expected) == expected);
    LIBCPP_ASSERT((expected | std::regex_traits<char_type>::__regex_word) == result);

    const bool matches_underscore = t.isctype('_', result);
    if (result != expected)
      assert(matches_underscore && "expected to match underscore");
    else
      assert(!matches_underscore && "should not match underscore");
}

int main()
{
//  if __regex_word is not distinct from all the classes, bad things happen
//  See https://bugs.llvm.org/show_bug.cgi?id=26476 for an example.
    LIBCPP_ASSERT((std::ctype_base::space  & std::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::print  & std::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::cntrl  & std::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::upper  & std::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::lower  & std::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::alpha  & std::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::digit  & std::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::punct  & std::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::xdigit & std::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::blank  & std::regex_traits<char>::__regex_word) == 0);

    test("d", std::ctype_base::digit);
    test("D", std::ctype_base::digit);
    test("d", std::ctype_base::digit, true);
    test("D", std::ctype_base::digit, true);

    test_w("w", std::ctype_base::alnum
              | std::ctype_base::upper | std::ctype_base::lower);
    test_w("W", std::ctype_base::alnum
              | std::ctype_base::upper | std::ctype_base::lower);
    test_w("w", std::ctype_base::alnum
              | std::ctype_base::upper | std::ctype_base::lower, true);
    test_w("W", std::ctype_base::alnum
              | std::ctype_base::upper | std::ctype_base::lower, true);

    test("s", std::ctype_base::space);
    test("S", std::ctype_base::space);
    test("s", std::ctype_base::space, true);
    test("S", std::ctype_base::space, true);

    test("alnum", std::ctype_base::alnum);
    test("AlNum", std::ctype_base::alnum);
    test("alnum", std::ctype_base::alnum, true);
    test("AlNum", std::ctype_base::alnum, true);

    test("alpha", std::ctype_base::alpha);
    test("Alpha", std::ctype_base::alpha);
    test("alpha", std::ctype_base::alpha, true);
    test("Alpha", std::ctype_base::alpha, true);

    test("blank", std::ctype_base::blank);
    test("Blank", std::ctype_base::blank);
    test("blank", std::ctype_base::blank, true);
    test("Blank", std::ctype_base::blank, true);

    test("cntrl", std::ctype_base::cntrl);
    test("Cntrl", std::ctype_base::cntrl);
    test("cntrl", std::ctype_base::cntrl, true);
    test("Cntrl", std::ctype_base::cntrl, true);

    test("digit", std::ctype_base::digit);
    test("Digit", std::ctype_base::digit);
    test("digit", std::ctype_base::digit, true);
    test("Digit", std::ctype_base::digit, true);

    test("digit", std::ctype_base::digit);
    test("DIGIT", std::ctype_base::digit);
    test("digit", std::ctype_base::digit, true);
    test("Digit", std::ctype_base::digit, true);

    test("graph", std::ctype_base::graph);
    test("GRAPH", std::ctype_base::graph);
    test("graph", std::ctype_base::graph, true);
    test("Graph", std::ctype_base::graph, true);

    test("lower", std::ctype_base::lower);
    test("LOWER", std::ctype_base::lower);
    test("lower", std::ctype_base::lower | std::ctype_base::alpha, true);
    test("Lower", std::ctype_base::lower | std::ctype_base::alpha, true);

    test("print", std::ctype_base::print);
    test("PRINT", std::ctype_base::print);
    test("print", std::ctype_base::print, true);
    test("Print", std::ctype_base::print, true);

    test("punct", std::ctype_base::punct);
    test("PUNCT", std::ctype_base::punct);
    test("punct", std::ctype_base::punct, true);
    test("Punct", std::ctype_base::punct, true);

    test("space", std::ctype_base::space);
    test("SPACE", std::ctype_base::space);
    test("space", std::ctype_base::space, true);
    test("Space", std::ctype_base::space, true);

    test("upper", std::ctype_base::upper);
    test("UPPER", std::ctype_base::upper);
    test("upper", std::ctype_base::upper | std::ctype_base::alpha, true);
    test("Upper", std::ctype_base::upper | std::ctype_base::alpha, true);

    test("xdigit", std::ctype_base::xdigit);
    test("XDIGIT", std::ctype_base::xdigit);
    test("xdigit", std::ctype_base::xdigit, true);
    test("Xdigit", std::ctype_base::xdigit, true);

    test("dig", std::ctype_base::mask());
    test("", std::ctype_base::mask());
    test("digits", std::ctype_base::mask());

    test(L"d", std::ctype_base::digit);
    test(L"D", std::ctype_base::digit);
    test(L"d", std::ctype_base::digit, true);
    test(L"D", std::ctype_base::digit, true);

    test_w(L"w", std::ctype_base::alnum
                      | std::ctype_base::upper | std::ctype_base::lower);
    test_w(L"W", std::ctype_base::alnum
                      | std::ctype_base::upper | std::ctype_base::lower);
    test_w(L"w", std::ctype_base::alnum
                      | std::ctype_base::upper | std::ctype_base::lower, true);
    test_w(L"W", std::ctype_base::alnum
                      | std::ctype_base::upper | std::ctype_base::lower, true);

    test(L"s", std::ctype_base::space);
    test(L"S", std::ctype_base::space);
    test(L"s", std::ctype_base::space, true);
    test(L"S", std::ctype_base::space, true);

    test(L"alnum", std::ctype_base::alnum);
    test(L"AlNum", std::ctype_base::alnum);
    test(L"alnum", std::ctype_base::alnum, true);
    test(L"AlNum", std::ctype_base::alnum, true);

    test(L"alpha", std::ctype_base::alpha);
    test(L"Alpha", std::ctype_base::alpha);
    test(L"alpha", std::ctype_base::alpha, true);
    test(L"Alpha", std::ctype_base::alpha, true);

    test(L"blank", std::ctype_base::blank);
    test(L"Blank", std::ctype_base::blank);
    test(L"blank", std::ctype_base::blank, true);
    test(L"Blank", std::ctype_base::blank, true);

    test(L"cntrl", std::ctype_base::cntrl);
    test(L"Cntrl", std::ctype_base::cntrl);
    test(L"cntrl", std::ctype_base::cntrl, true);
    test(L"Cntrl", std::ctype_base::cntrl, true);

    test(L"digit", std::ctype_base::digit);
    test(L"Digit", std::ctype_base::digit);
    test(L"digit", std::ctype_base::digit, true);
    test(L"Digit", std::ctype_base::digit, true);

    test(L"digit", std::ctype_base::digit);
    test(L"DIGIT", std::ctype_base::digit);
    test(L"digit", std::ctype_base::digit, true);
    test(L"Digit", std::ctype_base::digit, true);

    test(L"graph", std::ctype_base::graph);
    test(L"GRAPH", std::ctype_base::graph);
    test(L"graph", std::ctype_base::graph, true);
    test(L"Graph", std::ctype_base::graph, true);

    test(L"lower", std::ctype_base::lower);
    test(L"LOWER", std::ctype_base::lower);
    test(L"lower", std::ctype_base::lower | std::ctype_base::alpha, true);
    test(L"Lower", std::ctype_base::lower | std::ctype_base::alpha, true);

    test(L"print", std::ctype_base::print);
    test(L"PRINT", std::ctype_base::print);
    test(L"print", std::ctype_base::print, true);
    test(L"Print", std::ctype_base::print, true);

    test(L"punct", std::ctype_base::punct);
    test(L"PUNCT", std::ctype_base::punct);
    test(L"punct", std::ctype_base::punct, true);
    test(L"Punct", std::ctype_base::punct, true);

    test(L"space", std::ctype_base::space);
    test(L"SPACE", std::ctype_base::space);
    test(L"space", std::ctype_base::space, true);
    test(L"Space", std::ctype_base::space, true);

    test(L"upper", std::ctype_base::upper);
    test(L"UPPER", std::ctype_base::upper);
    test(L"upper", std::ctype_base::upper | std::ctype_base::alpha, true);
    test(L"Upper", std::ctype_base::upper | std::ctype_base::alpha, true);

    test(L"xdigit", std::ctype_base::xdigit);
    test(L"XDIGIT", std::ctype_base::xdigit);
    test(L"xdigit", std::ctype_base::xdigit, true);
    test(L"Xdigit", std::ctype_base::xdigit, true);

    test(L"dig", std::ctype_base::mask());
    test(L"", std::ctype_base::mask());
    test(L"digits", std::ctype_base::mask());
}
