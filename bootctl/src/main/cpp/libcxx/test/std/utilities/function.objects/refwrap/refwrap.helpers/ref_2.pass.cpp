//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// reference_wrapper

// template <ObjectType T> reference_wrapper<T> ref(reference_wrapper<T>t);

#include <functional>
#include <cassert>

#include "counting_predicates.hpp"

bool is5 ( int i ) { return i == 5; }

template <typename T>
bool call_pred ( T pred ) { return pred(5); }

int main()
{
    {
    int i = 0;
    std::reference_wrapper<int> r1 = std::ref(i);
    std::reference_wrapper<int> r2 = std::ref(r1);
    assert(&r2.get() == &i);
    }
    {
    unary_counting_predicate<bool(*)(int), int> cp(is5);
    assert(!cp(6));
    assert(cp.count() == 1);
    assert(call_pred(cp));
    assert(cp.count() == 1);
    assert(call_pred(std::ref(cp)));
    assert(cp.count() == 2);
    }
}
