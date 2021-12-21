//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Can't test the system lib because this test enables debug mode
// UNSUPPORTED: with_system_cxx_lib

// <list>

// template <InputIterator Iter>
//   iterator insert(const_iterator position, Iter first, Iter last);


#define _LIBCPP_DEBUG 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <list>
#include <cstdlib>
#include <cassert>
#include "test_iterators.h"

int main()
{
    {
        std::list<int> v(100);
        std::list<int> v2(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        std::list<int>::iterator i = v.insert(next(v2.cbegin(), 10),
                                        input_iterator<const int*>(a),
                                       input_iterator<const int*>(a+N));
        assert(false);
    }
}
