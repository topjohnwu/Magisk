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

// explicit operator bool() const;

#include <memory>
#include <cassert>

int main()
{
    {
    const std::shared_ptr<int> p(new int(32));
    assert(p);
    }
    {
    const std::shared_ptr<int> p;
    assert(!p);
    }
}
