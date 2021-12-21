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

// <mutex>

// struct once_flag;

// template<class Callable, class ...Args>
//   void call_once(once_flag& flag, Callable&& func, Args&&... args);

#include <mutex>
#include <thread>
#include <cassert>

#include "test_macros.h"

typedef std::chrono::milliseconds ms;

std::once_flag flg0;

int init0_called = 0;

void init0()
{
    std::this_thread::sleep_for(ms(250));
    ++init0_called;
}

void f0()
{
    std::call_once(flg0, init0);
}

std::once_flag flg3;

int init3_called = 0;
int init3_completed = 0;

void init3()
{
    ++init3_called;
    std::this_thread::sleep_for(ms(250));
    if (init3_called == 1)
        TEST_THROW(1);
    ++init3_completed;
}

void f3()
{
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
    {
        std::call_once(flg3, init3);
    }
    catch (...)
    {
    }
#endif
}

#if TEST_STD_VER >= 11

struct init1
{
    static int called;

    void operator()(int i) {called += i;}
};

int init1::called = 0;

std::once_flag flg1;

void f1()
{
    std::call_once(flg1, init1(), 1);
}

struct init2
{
    static int called;

    void operator()(int i, int j) const {called += i + j;}
};

int init2::called = 0;

std::once_flag flg2;

void f2()
{
    std::call_once(flg2, init2(), 2, 3);
    std::call_once(flg2, init2(), 4, 5);
}

#endif  // TEST_STD_VER >= 11

std::once_flag flg41;
std::once_flag flg42;

int init41_called = 0;
int init42_called = 0;

void init42();

void init41()
{
    std::this_thread::sleep_for(ms(250));
    ++init41_called;
}

void init42()
{
    std::this_thread::sleep_for(ms(250));
    ++init42_called;
}

void f41()
{
    std::call_once(flg41, init41);
    std::call_once(flg42, init42);
}

void f42()
{
    std::call_once(flg42, init42);
    std::call_once(flg41, init41);
}

#if TEST_STD_VER >= 11

class MoveOnly
{
#if !defined(__clang__)
   // GCC 4.8 complains about the following being private
public:
    MoveOnly(const MoveOnly&)
    {
    }
#else
    MoveOnly(const MoveOnly&);
#endif
public:
    MoveOnly() {}
    MoveOnly(MoveOnly&&) {}

    void operator()(MoveOnly&&)
    {
    }
};

class NonCopyable
{
#if !defined(__clang__)
   // GCC 4.8 complains about the following being private
public:
    NonCopyable(const NonCopyable&)
    {
    }
#else
    NonCopyable(const NonCopyable&);
#endif
public:
    NonCopyable() {}

    void operator()(int&) {}
};

// reference qualifiers on functions are a C++11 extension
struct RefQual
{
    int lv_called, rv_called;

    RefQual() : lv_called(0), rv_called(0) {}

    void operator()() & { ++lv_called; }
    void operator()() && { ++rv_called; }
};

#endif // TEST_STD_VER >= 11

int main()
{
    // check basic functionality
    {
        std::thread t0(f0);
        std::thread t1(f0);
        t0.join();
        t1.join();
        assert(init0_called == 1);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    // check basic exception safety
    {
        std::thread t0(f3);
        std::thread t1(f3);
        t0.join();
        t1.join();
        assert(init3_called == 2);
        assert(init3_completed == 1);
    }
#endif
    // check deadlock avoidance
    {
        std::thread t0(f41);
        std::thread t1(f42);
        t0.join();
        t1.join();
        assert(init41_called == 1);
        assert(init42_called == 1);
    }
#if TEST_STD_VER >= 11
    // check functors with 1 arg
    {
        std::thread t0(f1);
        std::thread t1(f1);
        t0.join();
        t1.join();
        assert(init1::called == 1);
    }
    // check functors with 2 args
    {
        std::thread t0(f2);
        std::thread t1(f2);
        t0.join();
        t1.join();
        assert(init2::called == 5);
    }
    {
        std::once_flag f;
        std::call_once(f, MoveOnly(), MoveOnly());
    }
    // check LWG2442: call_once() shouldn't DECAY_COPY()
    {
        std::once_flag f;
        int i = 0;
        std::call_once(f, NonCopyable(), i);
    }
// reference qualifiers on functions are a C++11 extension
    {
        std::once_flag f1, f2;
        RefQual rq;
        std::call_once(f1, rq);
        assert(rq.lv_called == 1);
        std::call_once(f2, std::move(rq));
        assert(rq.rv_called == 1);
    }
#endif  // TEST_STD_VER >= 11
}
