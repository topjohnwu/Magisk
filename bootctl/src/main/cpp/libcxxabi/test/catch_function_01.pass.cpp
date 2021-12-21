//===----------------------- catch_function_01.cpp ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Can you have a catch clause of array type that catches anything?

// GCC incorrectly allows function pointer to be caught by reference.
// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69372
// XFAIL: gcc
// UNSUPPORTED: libcxxabi-no-exceptions

#include <cassert>

template <class Tp>
bool can_convert(Tp) { return true; }

template <class>
bool can_convert(...) { return false; }

void f() {}

int main()
{
    typedef void Function();
    assert(!can_convert<Function&>(&f));
    assert(!can_convert<void*>(&f));
    try
    {
        throw f;     // converts to void (*)()
        assert(false);
    }
    catch (Function& b)  // can't catch void (*)()
    {
        assert(false);
    }
    catch (void*) // can't catch as void*
    {
        assert(false);
    }
    catch(Function*)
    {
    }
    catch (...)
    {
        assert(false);
    }
}
