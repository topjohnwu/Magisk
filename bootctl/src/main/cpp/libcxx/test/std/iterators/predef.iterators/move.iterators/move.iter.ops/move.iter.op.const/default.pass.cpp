//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// move_iterator

// move_iterator();
//
//  constexpr in C++17

#include <iterator>

#include "test_macros.h"
#include "test_iterators.h"

template <class It>
void
test()
{
    std::move_iterator<It> r;
    (void)r;
}

int main()
{
    test<input_iterator<char*> >();
    test<forward_iterator<char*> >();
    test<bidirectional_iterator<char*> >();
    test<random_access_iterator<char*> >();
    test<char*>();

#if TEST_STD_VER > 14
    {
    constexpr std::move_iterator<const char *> it;
    (void)it;
    }
#endif
}
