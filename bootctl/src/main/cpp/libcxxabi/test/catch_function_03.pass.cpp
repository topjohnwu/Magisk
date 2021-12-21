//===---------------------- catch_function_03.cpp -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Can a noexcept function pointer be caught by a non-noexcept catch clause?
// UNSUPPORTED: libcxxabi-no-exceptions, libcxxabi-no-noexcept-function-type

#include <cassert>

template<bool Noexcept> void f() noexcept(Noexcept) {}
template<bool Noexcept> using FnType = void() noexcept(Noexcept);

template<bool ThrowNoexcept, bool CatchNoexcept>
void check()
{
    try
    {
        auto *p = f<ThrowNoexcept>;
        throw p;
        assert(false);
    }
    catch (FnType<CatchNoexcept> *p)
    {
        assert(ThrowNoexcept || !CatchNoexcept);
        assert(p == &f<ThrowNoexcept>);
    }
    catch (...)
    {
        assert(!ThrowNoexcept && CatchNoexcept);
    }
}

void check_deep() {
    auto *p = f<true>;
    try
    {
        throw &p;
    }
    catch (FnType<false> **q)
    {
        assert(false);
    }
    catch (FnType<true> **q)
    {
    }
    catch (...)
    {
        assert(false);
    }
}

int main()
{
    check<false, false>();
    check<false, true>();
    check<true, false>();
    check<true, true>();
    check_deep();
}
