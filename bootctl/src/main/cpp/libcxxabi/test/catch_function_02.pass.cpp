//===---------------------- catch_function_02.cpp -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Can you have a catch clause of array type that catches anything?
// UNSUPPORTED: libcxxabi-no-exceptions

#include <cassert>

void f() {}

int main()
{
    typedef void Function();
    try
    {
        throw f;     // converts to void (*)()
        assert(false);
    }
    catch (Function b)  // equivalent to void (*)()
    {
    }
    catch (...)
    {
        assert(false);
    }
}
