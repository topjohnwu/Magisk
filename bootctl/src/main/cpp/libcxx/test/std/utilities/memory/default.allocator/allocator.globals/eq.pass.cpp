//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// allocator:

// template <class T1, class T2>
//   bool
//   operator==(const allocator<T1>&, const allocator<T2>&) throw();
//
// template <class T1, class T2>
//   bool
//   operator!=(const allocator<T1>&, const allocator<T2>&) throw();

#include <memory>
#include <cassert>

int main()
{
    std::allocator<int> a1;
    std::allocator<int> a2;
    assert(a1 == a2);
    assert(!(a1 != a2));
}
