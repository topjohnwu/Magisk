//===---------------------- catch_in_noexcept.cpp--------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, libcxxabi-no-exceptions

#include <exception>
#include <stdlib.h>
#include <assert.h>

struct A {};

// Despite being marked as noexcept, this function must have an EHT entry that
// is not 'cantunwind', so that the unwinder can correctly deal with the throw.
void f1() noexcept
{
    try {
        A a;
        throw a;
        assert(false);
    } catch (...) {
        assert(true);
        return;
    }
    assert(false);
}

int main()
{
    f1();
}
