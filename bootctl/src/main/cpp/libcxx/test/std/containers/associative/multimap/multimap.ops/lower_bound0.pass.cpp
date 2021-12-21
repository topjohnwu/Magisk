//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// XFAIL: c++98, c++03, c++11

// <map>

// class multimap

//       iterator lower_bound(const key_type& k);
// const_iterator lower_bound(const key_type& k) const;
//
//   The member function templates find, count, lower_bound, upper_bound, and
// equal_range shall not participate in overload resolution unless the
// qualified-id Compare::is_transparent is valid and denotes a type


#include <map>
#include <cassert>

#include "is_transparent.h"

int main()
{
    {
    typedef std::multimap<int, double, transparent_less> M;
    M example;
    assert(example.lower_bound(C2Int{5}) == example.end());
    }
    {
    typedef std::multimap<int, double, transparent_less_not_referenceable> M;
    M example;
    assert(example.lower_bound(C2Int{5}) == example.end());
    }
}
