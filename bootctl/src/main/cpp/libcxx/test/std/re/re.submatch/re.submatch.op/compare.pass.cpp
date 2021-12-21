//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator> class sub_match;

// template <class BiIter>
//     bool
//     operator==(const sub_match<BiIter>& lhs, const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator!=(const sub_match<BiIter>& lhs, const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator<(const sub_match<BiIter>& lhs, const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator<=(const sub_match<BiIter>& lhs, const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator>=(const sub_match<BiIter>& lhs, const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator>(const sub_match<BiIter>& lhs, const sub_match<BiIter>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool
//     operator==(const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool
//     operator!=(const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool
//     operator<(const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& lhs,
//               const sub_match<BiIter>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool
//     operator>(const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& lhs,
//               const sub_match<BiIter>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool operator>=(const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& lhs,
//                     const sub_match<BiIter>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool
//     operator<=(const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool
//     operator==(const sub_match<BiIter>& lhs,
//                const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool
//     operator!=(const sub_match<BiIter>& lhs,
//                const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool
//     operator<(const sub_match<BiIter>& lhs,
//               const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool operator>(const sub_match<BiIter>& lhs,
//                    const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool
//     operator>=(const sub_match<BiIter>& lhs,
//                const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& rhs);
//
// template <class BiIter, class ST, class SA>
//     bool
//     operator<=(const sub_match<BiIter>& lhs,
//                const basic_string<typename iterator_traits<BiIter>::value_type, ST, SA>& rhs);
//
// template <class BiIter>
//     bool
//     operator==(typename iterator_traits<BiIter>::value_type const* lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator!=(typename iterator_traits<BiIter>::value_type const* lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator<(typename iterator_traits<BiIter>::value_type const* lhs,
//               const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator>(typename iterator_traits<BiIter>::value_type const* lhs,
//               const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator>=(typename iterator_traits<BiIter>::value_type const* lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator<=(typename iterator_traits<BiIter>::value_type const* lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator==(const sub_match<BiIter>& lhs,
//                typename iterator_traits<BiIter>::value_type const* rhs);
//
// template <class BiIter>
//     bool
//     operator!=(const sub_match<BiIter>& lhs,
//                typename iterator_traits<BiIter>::value_type const* rhs);
//
// template <class BiIter>
//     bool
//     operator<(const sub_match<BiIter>& lhs,
//               typename iterator_traits<BiIter>::value_type const* rhs);
//
// template <class BiIter>
//     bool
//     operator>(const sub_match<BiIter>& lhs,
//               typename iterator_traits<BiIter>::value_type const* rhs);
//
// template <class BiIter>
//     bool
//     operator>=(const sub_match<BiIter>& lhs,
//                typename iterator_traits<BiIter>::value_type const* rhs);
//
// template <class BiIter>
//     bool
//     operator<=(const sub_match<BiIter>& lhs,
//                typename iterator_traits<BiIter>::value_type const* rhs);
//
// template <class BiIter>
//     bool
//     operator==(typename iterator_traits<BiIter>::value_type const& lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator!=(typename iterator_traits<BiIter>::value_type const& lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator<(typename iterator_traits<BiIter>::value_type const& lhs,
//               const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator>(typename iterator_traits<BiIter>::value_type const& lhs,
//               const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator>=(typename iterator_traits<BiIter>::value_type const& lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator<=(typename iterator_traits<BiIter>::value_type const& lhs,
//                const sub_match<BiIter>& rhs);
//
// template <class BiIter>
//     bool
//     operator==(const sub_match<BiIter>& lhs,
//                typename iterator_traits<BiIter>::value_type const& rhs);
//
// template <class BiIter>
//     bool
//     operator!=(const sub_match<BiIter>& lhs,
//                typename iterator_traits<BiIter>::value_type const& rhs);
//
// template <class BiIter>
//     bool
//     operator<(const sub_match<BiIter>& lhs,
//               typename iterator_traits<BiIter>::value_type const& rhs);
//
// template <class BiIter>
//     bool
//     operator>(const sub_match<BiIter>& lhs,
//               typename iterator_traits<BiIter>::value_type const& rhs);
//
// template <class BiIter>
//     bool
//     operator>=(const sub_match<BiIter>& lhs,
//                typename iterator_traits<BiIter>::value_type const& rhs);
//
// template <class BiIter>
//     bool
//     operator<=(const sub_match<BiIter>& lhs,
//                typename iterator_traits<BiIter>::value_type const& rhs);

#include <regex>
#include <cassert>
#include "test_macros.h"

template <class CharT>
void
test(const std::basic_string<CharT>& x, const std::basic_string<CharT>& y, bool doCStrTests = true)
{
    typedef std::basic_string<CharT> string;
    typedef std::sub_match<typename string::const_iterator> sub_match;
    sub_match sm1;
    sm1.first = x.begin();
    sm1.second = x.end();
    sm1.matched = true;
    sub_match sm2;
    sm2.first = y.begin();
    sm2.second = y.end();
    sm2.matched = true;
    assert((sm1 == sm2) == (x == y));
    assert((sm1 != sm2) == (x != y));
    assert((sm1 < sm2) == (x < y));
    assert((sm1 > sm2) == (x > y));
    assert((sm1 <= sm2) == (x <= y));
    assert((sm1 >= sm2) == (x >= y));
    assert((x == sm2) == (x == y));
    assert((x != sm2) == (x != y));
    assert((x < sm2) == (x < y));
    assert((x > sm2) == (x > y));
    assert((x <= sm2) == (x <= y));
    assert((x >= sm2) == (x >= y));
    assert((sm1 == y) == (x == y));
    assert((sm1 != y) == (x != y));
    assert((sm1 < y) == (x < y));
    assert((sm1 > y) == (x > y));
    assert((sm1 <= y) == (x <= y));
    assert((sm1 >= y) == (x >= y));
    if (doCStrTests) {
        assert((x.c_str() == sm2) == (x == y));
        assert((x.c_str() != sm2) == (x != y));
        assert((x.c_str() < sm2) == (x < y));
        assert((x.c_str() > sm2) == (x > y));
        assert((x.c_str() <= sm2) == (x <= y));
        assert((x.c_str() >= sm2) == (x >= y));
        assert((sm1 == y.c_str()) == (x == y));
        assert((sm1 != y.c_str()) == (x != y));
        assert((sm1 < y.c_str()) == (x < y));
        assert((sm1 > y.c_str()) == (x > y));
        assert((sm1 <= y.c_str()) == (x <= y));
        assert((sm1 >= y.c_str()) == (x >= y));
        }
    assert((x[0] == sm2) == (string(1, x[0]) == y));
    assert((x[0] != sm2) == (string(1, x[0]) != y));
    assert((x[0] < sm2) == (string(1, x[0]) < y));
    assert((x[0] > sm2) == (string(1, x[0]) > y));
    assert((x[0] <= sm2) == (string(1, x[0]) <= y));
    assert((x[0] >= sm2) == (string(1, x[0]) >= y));
    assert((sm1 == y[0]) == (x == string(1, y[0])));
    assert((sm1 != y[0]) == (x != string(1, y[0])));
    assert((sm1 < y[0]) == (x < string(1, y[0])));
    assert((sm1 > y[0]) == (x > string(1, y[0])));
    assert((sm1 <= y[0]) == (x <= string(1, y[0])));
    assert((sm1 >= y[0]) == (x >= string(1, y[0])));
}

int main()
{
    test(std::string("123"), std::string("123"));
    test(std::string("1234"), std::string("123"));
    test(std::wstring(L"123"), std::wstring(L"123"));
    test(std::wstring(L"1234"), std::wstring(L"123"));
    test(std::string("123\000" "56", 6), std::string("123\000" "56", 6), false);
    test(std::wstring(L"123\000" L"56", 6), std::wstring(L"123\000" L"56", 6), false);
}
