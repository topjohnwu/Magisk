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

// template <ObjectType T> reference_wrapper<const T> cref(reference_wrapper<T> t);

#include <functional>
#include <cassert>

int main()
{
    const int i = 0;
    std::reference_wrapper<const int> r1 = std::cref(i);
    std::reference_wrapper<const int> r2 = std::cref(r1);
    assert(&r2.get() == &i);
}
