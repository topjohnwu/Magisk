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

// T* operator->() const;

#include <memory>
#include <utility>
#include <cassert>

int main()
{
    const std::shared_ptr<std::pair<int, int> > p(new std::pair<int, int>(3, 4));
    assert(p->first == 3);
    assert(p->second == 4);
    p->first = 5;
    p->second = 6;
    assert(p->first == 5);
    assert(p->second == 6);
}
