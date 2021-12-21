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

// class future<R>

// template <class Clock, class Duration>
//   future_status
//   wait_until(const chrono::time_point<Clock, Duration>& abs_time) const;

#include <future>
#include <atomic>
#include <cassert>

enum class WorkerThreadState { Uninitialized, AllowedToRun, Exiting };
typedef std::chrono::milliseconds ms;

std::atomic<WorkerThreadState> thread_state(WorkerThreadState::Uninitialized);

void set_worker_thread_state(WorkerThreadState state)
{
    thread_state.store(state, std::memory_order_relaxed);
}

void wait_for_worker_thread_state(WorkerThreadState state)
{
    while (thread_state.load(std::memory_order_relaxed) != state);
}

void func1(std::promise<int> p)
{
    wait_for_worker_thread_state(WorkerThreadState::AllowedToRun);
    p.set_value(3);
    set_worker_thread_state(WorkerThreadState::Exiting);
}

int j = 0;

void func3(std::promise<int&> p)
{
    wait_for_worker_thread_state(WorkerThreadState::AllowedToRun);
    j = 5;
    p.set_value(j);
    set_worker_thread_state(WorkerThreadState::Exiting);
}

void func5(std::promise<void> p)
{
    wait_for_worker_thread_state(WorkerThreadState::AllowedToRun);
    p.set_value();
    set_worker_thread_state(WorkerThreadState::Exiting);
}

int main()
{
    typedef std::chrono::high_resolution_clock Clock;
    {
        typedef int T;
        std::promise<T> p;
        std::future<T> f = p.get_future();
        std::thread(func1, std::move(p)).detach();
        assert(f.valid());
        assert(f.wait_until(Clock::now() + ms(10)) == std::future_status::timeout);
        assert(f.valid());

        // allow the worker thread to produce the result and wait until the worker is done
        set_worker_thread_state(WorkerThreadState::AllowedToRun);
        wait_for_worker_thread_state(WorkerThreadState::Exiting);

        assert(f.wait_until(Clock::now() + ms(10)) == std::future_status::ready);
        assert(f.valid());
        Clock::time_point t0 = Clock::now();
        f.wait();
        Clock::time_point t1 = Clock::now();
        assert(f.valid());
        assert(t1-t0 < ms(5));
    }
    {
        typedef int& T;
        std::promise<T> p;
        std::future<T> f = p.get_future();
        std::thread(func3, std::move(p)).detach();
        assert(f.valid());
        assert(f.wait_until(Clock::now() + ms(10)) == std::future_status::timeout);
        assert(f.valid());

        // allow the worker thread to produce the result and wait until the worker is done
        set_worker_thread_state(WorkerThreadState::AllowedToRun);
        wait_for_worker_thread_state(WorkerThreadState::Exiting);

        assert(f.wait_until(Clock::now() + ms(10)) == std::future_status::ready);
        assert(f.valid());
        Clock::time_point t0 = Clock::now();
        f.wait();
        Clock::time_point t1 = Clock::now();
        assert(f.valid());
        assert(t1-t0 < ms(5));
    }
    {
        typedef void T;
        std::promise<T> p;
        std::future<T> f = p.get_future();
        std::thread(func5, std::move(p)).detach();
        assert(f.valid());
        assert(f.wait_until(Clock::now() + ms(10)) == std::future_status::timeout);
        assert(f.valid());

        // allow the worker thread to produce the result and wait until the worker is done
        set_worker_thread_state(WorkerThreadState::AllowedToRun);
        wait_for_worker_thread_state(WorkerThreadState::Exiting);

        assert(f.wait_until(Clock::now() + ms(10)) == std::future_status::ready);
        assert(f.valid());
        Clock::time_point t0 = Clock::now();
        f.wait();
        Clock::time_point t1 = Clock::now();
        assert(f.valid());
        assert(t1-t0 < ms(5));
    }
}
