//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class money_base
// {
// public:
//     enum part {none, space, symbol, sign, value};
//     struct pattern {char field[4];};
// };

#include <locale>
#include <cassert>

int main()
{
    std::money_base mb; ((void)mb);
    static_assert(std::money_base::none == 0, "");
    static_assert(std::money_base::space == 1, "");
    static_assert(std::money_base::symbol == 2, "");
    static_assert(std::money_base::sign == 3, "");
    static_assert(std::money_base::value == 4, "");
    static_assert(sizeof(std::money_base::pattern) == 4, "");
    std::money_base::pattern p;
    p.field[0] = std::money_base::none;
}
