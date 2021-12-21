//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// reverse_iterator

// constexpr reverse_iterator();
//
// constexpr in C++17

#include <iterator>

#include "test_macros.h"
#include "test_iterators.h"

template <class It>
void
test()
{
    std::reverse_iterator<It> r;
    (void)r;
}

int main()
{
    test<bidirectional_iterator<const char*> >();
    test<random_access_iterator<char*> >();
    test<char*>();
    test<const char*>();

#if TEST_STD_VER > 14
    {
        constexpr std::reverse_iterator<const char *> it;
        (void)it;
    }
#endif
}
