//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// shared_ptr

// template<class T, class U> bool operator==(const shared_ptr<T>& a, const shared_ptr<U>& b);
// template<class T, class U> bool operator!=(const shared_ptr<T>& a, const shared_ptr<U>& b);

#include <memory>
#include <cassert>

void do_nothing(int*) {}

int main()
{
    int* ptr1(new int);
    int* ptr2(new int);
    const std::shared_ptr<int> p1(ptr1);
    const std::shared_ptr<int> p2(ptr2);
    const std::shared_ptr<int> p3(ptr2, do_nothing);
    assert(p1 != p2);
    assert(p2 == p3);
}
