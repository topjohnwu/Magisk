// Cached thread pool implementation

#include <utils.hpp>

#include <daemon.hpp>
#include <memory>

using namespace std;

#ifndef PTHREAD_COND_INITIALIZER_MONOTONIC_NP
#define PTHREAD_COND_INITIALIZER_MONOTONIC_NP  { { 1 << 1 } }
#endif

#define THREAD_IDLE_MAX_SEC 60
#define CORE_POOL_SIZE 3

using Task = std::function<void()>;

// idle thread should be waited only by exec_task and notified only by thread_pool_loop
static atomic_signed_lock_free idle_threads = 0;
static atomic_unsigned_lock_free total_threads = 0;
// pending_task should be waited only by thread_pool_loop and notified only by exec_task
static atomic<Task *> pending_task = nullptr;

static void reset_pool() {
    clear_poll();
    idle_threads.store(0, memory_order_relaxed);
    total_threads.store(0, memory_order_relaxed);
    delete pending_task.exchange(nullptr, memory_order_relaxed);
}

static void *thread_pool_loop(void *const) {
    idle_threads.fetch_add(1, memory_order_relaxed);
    total_threads.fetch_add(1, memory_order_relaxed);

    pthread_atfork(nullptr, nullptr, &reset_pool);

    // Block all signals
    sigset_t mask;
    sigfillset(&mask);

    bool timeout = false;
    for (;;) {
        // Restore sigmask
        pthread_sigmask(SIG_SETMASK, &mask, nullptr);
        Task *local_task = pending_task.exchange(nullptr, memory_order_acq_rel);
        if (!local_task) {
            // someone else consumes the task
            if (timeout) {
                if (total_threads.fetch_sub(1, memory_order_acq_rel) > CORE_POOL_SIZE) {
                    idle_threads.fetch_sub(1, memory_order_release);
                    return nullptr;
                } else {
                    total_threads.fetch_add(1, memory_order_release);
                }
            }
            // wait running thread changed, which indicates a job is added
            timeval tv{};
            gettimeofday(&tv, nullptr);
            auto sleep_time = tv.tv_sec;
            pending_task.wait(nullptr, memory_order_acquire);
            gettimeofday(&tv, nullptr);
            timeout = tv.tv_sec - sleep_time > THREAD_IDLE_MAX_SEC;
            // notified, try to fetch pending task again
            continue;
        }
        // successfully fetch a task
        timeout = false;
        // the following is done on exec_task
        // running_threads.fetch_add(1, memory_order_release);
        // idle_threads.fetch_sub(1, memory_order_release);
        // notify for potential new thread
        idle_threads.notify_one();
        (*local_task)();
        delete local_task;
        // forked from this thread
        if (getpid() == gettid()) pthread_exit(nullptr);
        idle_threads.fetch_add(1, memory_order_acq_rel);
        idle_threads.notify_one();
    }
}

void exec_task(Task &&task) {
    if (!task) return;
    auto desired_task = new Task(std::move(task));
    for (;;) {
        auto num_thread = idle_threads.load(memory_order_relaxed);
        // if pending_task == null_task: pending_task = desired_task
        // else: null_task = pending_task
        Task *null_task = nullptr;
        if (pending_task.compare_exchange_strong(null_task, desired_task, memory_order_release,
                                                 memory_order_relaxed)) {
            break;
        } else {
            // some exec_task is still running, wait active thread change,
            // which indicates a pending task is consumed.
            idle_threads.wait(num_thread, memory_order_acquire);
        }
    }
    // starting from here, there's must only be one exec_task
    if (idle_threads.fetch_sub(1, memory_order_acq_rel) == 0) {
        // no available_threads, new one
        new_daemon_thread(thread_pool_loop, nullptr);
    }
    pending_task.notify_all();
}
