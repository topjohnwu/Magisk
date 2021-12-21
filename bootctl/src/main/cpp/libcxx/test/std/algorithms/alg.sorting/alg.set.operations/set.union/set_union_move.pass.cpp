//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator InIter1, InputIterator InIter2, typename OutIter,
//          CopyConstructible Compare>
//   requires OutputIterator<OutIter, InIter1::reference>
//         && OutputIterator<OutIter, InIter2::reference>
//         && Predicate<Compare, InIter1::value_type, InIter2::value_type>
//         && Predicate<Compare, InIter2::value_type, InIter1::value_type>
//   OutIter
//   set_union(InIter1 first1, InIter1 last1, InIter2 first2, InIter2 last2,
//             OutIter result, Compare comp);

// UNSUPPORTED: c++98, c++03

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>

#include "MoveOnly.h"


int main()
{
    std::vector<MoveOnly> lhs, rhs;
    lhs.push_back(MoveOnly(2));
    rhs.push_back(MoveOnly(2));

    std::vector<MoveOnly> res;
    std::set_union(std::make_move_iterator(lhs.begin()),
                   std::make_move_iterator(lhs.end()),
                   std::make_move_iterator(rhs.begin()),
                   std::make_move_iterator(rhs.end()), std::back_inserter(res));

    assert(res.size() == 1);
    assert(res[0].get() == 2);
}
