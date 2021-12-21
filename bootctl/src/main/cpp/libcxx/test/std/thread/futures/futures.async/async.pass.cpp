//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: c++98, c++03

// <future>

// template <class F, class... Args>
//     future<typename result_of<F(Args...)>::type>
//     async(F&& f, Args&&... args);

// template <class F, class... Args>
//     future<typename result_of<F(Args...)>::type>
//     async(launch policy, F&& f, Args&&... args);


#include <future>
#include <atomic>
#include <memory>
#include <cassert>

#include "test_macros.h"

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds ms;

std::atomic_bool invoked = ATOMIC_VAR_INIT(false);

int f0()
{
    invoked = true;
    std::this_thread::sleep_for(ms(200));
    return 3;
}

int i = 0;

int& f1()
{
    invoked = true;
    std::this_thread::sleep_for(ms(200));
    return i;
}

void f2()
{
    invoked = true;
    std::this_thread::sleep_for(ms(200));
}

std::unique_ptr<int> f3(int j)
{
    invoked = true;
    std::this_thread::sleep_for(ms(200));
    return std::unique_ptr<int>(new int(j));
}

std::unique_ptr<int> f4(std::unique_ptr<int>&& p)
{
    invoked = true;
    std::this_thread::sleep_for(ms(200));
    return std::move(p);
}

void f5(int j)
{
    std::this_thread::sleep_for(ms(200));
    ((void)j);
    TEST_THROW(j);
}

template <class Ret, class CheckLamdba, class ...Args>
void test(CheckLamdba&& getAndCheckFn, bool IsDeferred, Args&&... args) {
    // Reset global state.
    invoked = false;

    // Create the future and wait
    std::future<Ret> f = std::async(std::forward<Args>(args)...);
    std::this_thread::sleep_for(ms(300));

    // Check that deferred async's have not invoked the function.
    assert(invoked == !IsDeferred);

    // Time the call to f.get() and check that the returned value matches
    // what is expected.
    Clock::time_point t0 = Clock::now();
    assert(getAndCheckFn(f));
    Clock::time_point t1 = Clock::now();

    // If the async is deferred it should take more than 100ms, otherwise
    // it should take less than 100ms.
    if (IsDeferred) {
        assert(t1-t0 > ms(100));
    } else {
        assert(t1-t0 < ms(100));
    }
}

int main()
{
    // The default launch policy is implementation defined. libc++ defines
    // it to be std::launch::async.
    bool DefaultPolicyIsDeferred = false;
    bool DPID = DefaultPolicyIsDeferred;

    std::launch AnyPolicy = std::launch::async | std::launch::deferred;
    LIBCPP_ASSERT(AnyPolicy == std::launch::any);

    {
        auto checkInt = [](std::future<int>& f) { return f.get() == 3; };
        test<int>(checkInt, DPID,  f0);
        test<int>(checkInt, false, std::launch::async, f0);
        test<int>(checkInt, true,  std::launch::deferred, f0);
        test<int>(checkInt, DPID,  AnyPolicy, f0);
    }
    {
        auto checkIntRef = [&](std::future<int&>& f) { return &f.get() == &i; };
        test<int&>(checkIntRef, DPID,  f1);
        test<int&>(checkIntRef, false, std::launch::async, f1);
        test<int&>(checkIntRef, true,  std::launch::deferred, f1);
        test<int&>(checkIntRef, DPID,  AnyPolicy, f1);
    }
    {
        auto checkVoid = [](std::future<void>& f) { f.get(); return true; };
        test<void>(checkVoid, DPID,  f2);
        test<void>(checkVoid, false, std::launch::async, f2);
        test<void>(checkVoid, true,  std::launch::deferred, f2);
        test<void>(checkVoid, DPID,  AnyPolicy, f2);
    }
    {
        using Ret = std::unique_ptr<int>;
        auto checkUPtr = [](std::future<Ret>& f) { return *f.get() == 3; };
        test<Ret>(checkUPtr, DPID, f3, 3);
        test<Ret>(checkUPtr, DPID, f4, std::unique_ptr<int>(new int(3)));
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        std::future<void> f = std::async(f5, 3);
        std::this_thread::sleep_for(ms(300));
        try { f.get(); assert (false); } catch ( int ) {}
    }
    {
        std::future<void> f = std::async(std::launch::deferred, f5, 3);
        std::this_thread::sleep_for(ms(300));
        try { f.get(); assert (false); } catch ( int ) {}
    }
#endif
}
