//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// void declare_reachable(void* p);
// template <class T> T* undeclare_reachable(T* p);

#include <memory>
#include <cassert>

int main()
{
    int* p = new int;
    std::declare_reachable(p);
    assert(std::undeclare_reachable(p) == p);
    delete p;
}
