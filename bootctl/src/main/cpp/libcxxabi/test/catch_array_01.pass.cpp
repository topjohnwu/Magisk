//===---------------------- catch_array_01.cpp ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Can you have a catch clause of array type that catches anything?

// GCC incorrectly allows array types to be caught by reference.
// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69372
// XFAIL: gcc
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
    catch (Array& b)  // can't catch char*
    {
        assert(false);
    }
    catch (...)
    {
    }
}
