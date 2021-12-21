//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class time_base
// {
// public:
//     enum dateorder {no_order, dmy, mdy, ymd, ydm};
// };

#include <locale>
#include <cassert>

int main()
{
    std::time_base::dateorder d = std::time_base::no_order;
    ((void)d); // Prevent unused warning
    assert(std::time_base::no_order == 0);
    assert(std::time_base::dmy == 1);
    assert(std::time_base::mdy == 2);
    assert(std::time_base::ymd == 3);
    assert(std::time_base::ydm == 4);
}
