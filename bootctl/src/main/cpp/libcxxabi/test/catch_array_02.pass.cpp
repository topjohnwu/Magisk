//===---------------------- catch_array_02.cpp ----------------------------===//
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

int main()
{
    typedef char Array[4];
    Array a = {'H', 'i', '!', 0};
    try
    {
        throw a;  // converts to char*
        assert(false);
    }
    catch (Array b)  // equivalent to char*
    {
    }
    catch (...)
    {
        assert(false);
    }
}
