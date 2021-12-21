//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class locale::id
// {
// public:
//     id();
//     void operator=(const id&) = delete;
//     id(const id&) = delete;
// };

// This test isn't portable

#include <locale>
#include <cassert>

std::locale::id id0;
std::locale::id id2;
std::locale::id id1;

int main()
{
    long id = id0.__get();
    assert(id0.__get() == id+0);
    assert(id0.__get() == id+0);
    assert(id0.__get() == id+0);
    assert(id1.__get() == id+1);
    assert(id1.__get() == id+1);
    assert(id1.__get() == id+1);
    assert(id2.__get() == id+2);
    assert(id2.__get() == id+2);
    assert(id2.__get() == id+2);
    assert(id0.__get() == id+0);
    assert(id0.__get() == id+0);
    assert(id0.__get() == id+0);
    assert(id1.__get() == id+1);
    assert(id1.__get() == id+1);
    assert(id1.__get() == id+1);
    assert(id2.__get() == id+2);
    assert(id2.__get() == id+2);
    assert(id2.__get() == id+2);
}
