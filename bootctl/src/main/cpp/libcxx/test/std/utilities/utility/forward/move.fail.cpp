//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// test move

#include <utility>
#include <cassert>

struct move_only {
    move_only() {}
    move_only(move_only&&) = default;
    move_only& operator=(move_only&&) = default;
};

move_only source() {return move_only();}
const move_only csource() {return move_only();}

void test(move_only) {}

int main()
{
    move_only a;
    const move_only ca = move_only();

    test(std::move(ca)); // c
}
