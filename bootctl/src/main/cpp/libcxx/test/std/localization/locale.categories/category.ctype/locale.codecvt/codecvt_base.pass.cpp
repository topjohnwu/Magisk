//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class codecvt_base
// {
// public:
//     enum result {ok, partial, error, noconv};
// };

#include <locale>
#include <cassert>

int main()
{
    assert(std::codecvt_base::ok == 0);
    assert(std::codecvt_base::partial == 1);
    assert(std::codecvt_base::error == 2);
    assert(std::codecvt_base::noconv == 3);
}
