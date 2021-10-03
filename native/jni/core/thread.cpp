// Cached thread pool implementation

#include <utils.hpp>

#include <daemon.hpp>
#include <memory>

using namespace std;

#define THREAD_IDLE_MAX_SEC 60
#define CORE_POOL_SIZE 3

using Task = std::function<void()>;

// idle thread should be waited only by exec and notified only by loop
static atomic_int idle_threads = 0;
// running thread should be waited only by loop and notified only by exec
static atomic_int running_threads = 0;
static shared_ptr<Task> pending_task = nullptr;

static void *thread_pool_loop(void *const) {
    idle_threads.fetch_add(1, memory_order_relaxed);
    // Block all signals
    sigset_t mask;
    sigfillset(&mask);

    bool timeout = false;
    for (;;) {
        // Restore sigmask
        pthread_sigmask(SIG_SETMASK, &mask, nullptr);
        auto num_thread = running_threads.load(memory_order_acquire);
        std::shared_ptr<Task> null_task = nullptr;
        std::shared_ptr<Task> local_task =
                std::atomic_exchange_explicit(&pending_task, null_task, memory_order_acq_rel);
        if (!local_task) {
            // someone else consumes the task
            if (timeout) {
                auto idl = idle_threads.fetch_sub(1, memory_order_acq_rel);
                if (idl > 1 && (num_thread + idl) > CORE_POOL_SIZE)
                    return nullptr;
                else
                    idle_threads.fetch_add(1, memory_order_release);
            }
            // wait running thread changed, which indicates a job is added
            timeval tv{};
            gettimeofday(&tv, nullptr);
            auto sleep_time = tv.tv_sec;
            running_threads.wait(num_thread, memory_order_acquire);
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
        running_threads.fetch_sub(1, memory_order_relaxed);
        idle_threads.fetch_add(1, memory_order_acq_rel);
        idle_threads.notify_one();
    }
}

void exec_task(Task &&task) {
    if (!task) return;
    auto desired_task = make_shared<Task>(std::move(task));
    for (;;) {
        shared_ptr<Task> null_task = nullptr;
        auto num_thread = idle_threads.load(memory_order_relaxed);
        // if pending_task == null_task: pending_task = desired_task
        // else: null_task = pending_task
        if (atomic_compare_exchange_strong_explicit(&pending_task, &null_task, desired_task,
                                                    memory_order_release,
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
    running_threads.fetch_add(1, memory_order_release);
    running_threads.notify_all();
}
