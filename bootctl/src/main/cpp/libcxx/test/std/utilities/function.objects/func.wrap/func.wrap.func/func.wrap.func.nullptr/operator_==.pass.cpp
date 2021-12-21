//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// class function<R(ArgTypes...)>

// template <MoveConstructible R, MoveConstructible ... ArgTypes>
//   bool operator==(const function<R(ArgTypes...)>&, nullptr_t);
//
// template <MoveConstructible R, MoveConstructible ... ArgTypes>
//   bool operator==(nullptr_t, const function<R(ArgTypes...)>&);
//
// template <MoveConstructible R, MoveConstructible ... ArgTypes>
//   bool operator!=(const function<R(ArgTypes...)>&, nullptr_t);
//
// template <MoveConstructible  R, MoveConstructible ... ArgTypes>
//   bool operator!=(nullptr_t, const function<R(ArgTypes...)>&);

#include <functional>
#include <cassert>

int g(int) {return 0;}

int main()
{
    {
    std::function<int(int)> f;
    assert(f == nullptr);
    assert(nullptr == f);
    f = g;
    assert(f != nullptr);
    assert(nullptr != f);
    }
}
