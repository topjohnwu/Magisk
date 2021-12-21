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

// <thread>

// class thread

// template <class F, class ...Args> thread(F&& f, Args&&... args);

// UNSUPPORTED: sanitizer-new-delete

#include <thread>
#include <new>
#include <atomic>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

std::atomic<unsigned> throw_one(0xFFFF);
std::atomic<unsigned> outstanding_new(0);


void* operator new(std::size_t s) TEST_THROW_SPEC(std::bad_alloc)
{
    if (throw_one == 0)
        TEST_THROW(std::bad_alloc());
    --throw_one;
    ++outstanding_new;
    void* ret = std::malloc(s);
    if (!ret) std::abort(); // placate MSVC's unchecked malloc warning
    return ret;
}

void  operator delete(void* p) TEST_NOEXCEPT
{
    --outstanding_new;
    std::free(p);
}

bool f_run = false;

void f()
{
    f_run = true;
}

class G
{
    int alive_;
public:
    static int n_alive;
    static bool op_run;

    G() : alive_(1) {++n_alive;}
    G(const G& g) : alive_(g.alive_) {++n_alive;}
    ~G() {alive_ = 0; --n_alive;}

    void operator()()
    {
        assert(alive_ == 1);
        assert(n_alive >= 1);
        op_run = true;
    }

    void operator()(int i, double j)
    {
        assert(alive_ == 1);
        assert(n_alive >= 1);
        assert(i == 5);
        assert(j == 5.5);
        op_run = true;
    }
};

int G::n_alive = 0;
bool G::op_run = false;

#if TEST_STD_VER >= 11

class MoveOnly
{
    MoveOnly(const MoveOnly&);
public:
    MoveOnly() {}
    MoveOnly(MoveOnly&&) {}

    void operator()(MoveOnly&&)
    {
    }
};

#endif

// Test throwing std::bad_alloc
//-----------------------------
// Concerns:
//  A Each allocation performed during thread construction should be performed
//    in the parent thread so that std::terminate is not called if
//    std::bad_alloc is thrown by new.
//  B std::thread's constructor should properly handle exceptions and not leak
//    memory.
// Plan:
//  1 Create a thread and count the number of allocations, 'N', it performs.
//  2 For each allocation performed run a test where that allocation throws.
//    2.1 check that the exception can be caught in the parent thread.
//    2.2 Check that the functor has not been called.
//    2.3 Check that no memory allocated by the creation of the thread is leaked.
//  3 Finally check that a thread runs successfully if we throw after 'N+1'
//    allocations.
void test_throwing_new_during_thread_creation() {
#ifndef TEST_HAS_NO_EXCEPTIONS
    throw_one = 0xFFF;
    {
        std::thread t(f);
        t.join();
    }
    const int numAllocs = 0xFFF - throw_one;
    // i <= numAllocs means the last iteration is expected not to throw.
    for (int i=0; i <= numAllocs; ++i) {
        throw_one = i;
        f_run = false;
        unsigned old_outstanding = outstanding_new;
        try {
            std::thread t(f);
            assert(i == numAllocs); // Only final iteration will not throw.
            t.join();
            assert(f_run);
        } catch (std::bad_alloc const&) {
            assert(i < numAllocs);
            assert(!f_run); // (2.2)
        }
        assert(old_outstanding == outstanding_new); // (2.3)
    }
    f_run = false;
    throw_one = 0xFFF;
#endif
}

int main()
{
    test_throwing_new_during_thread_creation();
    {
        std::thread t(f);
        t.join();
        assert(f_run == true);
    }

    {
        assert(G::n_alive == 0);
        assert(!G::op_run);
        {
            G g;
            std::thread t(g);
            t.join();
        }
        assert(G::n_alive == 0);
        assert(G::op_run);
    }
    G::op_run = false;
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        try
        {
            throw_one = 0;
            assert(G::n_alive == 0);
            assert(!G::op_run);
            std::thread t((G()));
            assert(false);
        }
        catch (...)
        {
            throw_one = 0xFFFF;
            assert(G::n_alive == 0);
            assert(!G::op_run);
        }
    }
#endif
#if TEST_STD_VER >= 11
    {
        assert(G::n_alive == 0);
        assert(!G::op_run);
        {
            G g;
            std::thread t(g, 5, 5.5);
            t.join();
        }
        assert(G::n_alive == 0);
        assert(G::op_run);
    }
    {
        std::thread t = std::thread(MoveOnly(), MoveOnly());
        t.join();
    }
#endif
}
