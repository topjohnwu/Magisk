// Cached thread pool implementation

#include <base.hpp>

#include <core.hpp>

using namespace std;

#define THREAD_IDLE_MAX_SEC 60
#define CORE_POOL_SIZE 3

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t send_task = PTHREAD_COND_INITIALIZER_MONOTONIC_NP;
static pthread_cond_t recv_task = PTHREAD_COND_INITIALIZER_MONOTONIC_NP;

// The following variables should be guarded by lock
static int idle_threads = 0;
static int total_threads = 0;
static function<void()> pending_task;

static void operator+=(timespec &a, const timespec &b) {
    a.tv_sec += b.tv_sec;
    a.tv_nsec += b.tv_nsec;
    if (a.tv_nsec >= 1000000000L) {
        a.tv_sec++;
        a.tv_nsec -= 1000000000L;
    }
}

static void reset_pool() {
    clear_poll();
    pthread_mutex_unlock(&lock);
    pthread_mutex_destroy(&lock);
    pthread_mutex_init(&lock, nullptr);
    pthread_cond_destroy(&send_task);
    send_task = PTHREAD_COND_INITIALIZER_MONOTONIC_NP;
    pthread_cond_destroy(&recv_task);
    recv_task = PTHREAD_COND_INITIALIZER_MONOTONIC_NP;
    idle_threads = 0;
    total_threads = 0;
    pending_task = nullptr;
}

static void *thread_pool_loop(void * const is_core_pool) {
    // Block all signals
    sigset_t mask;
    sigfillset(&mask);

    for (;;) {
        // Restore sigmask
        pthread_sigmask(SIG_SETMASK, &mask, nullptr);
        function<void()> local_task;
        {
            mutex_guard g(lock);
            ++idle_threads;
            if (!pending_task) {
                if (is_core_pool) {
                    pthread_cond_wait(&send_task, &lock);
                } else {
                    timespec ts;
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    ts += { THREAD_IDLE_MAX_SEC, 0 };
                    if (pthread_cond_timedwait(&send_task, &lock, &ts) == ETIMEDOUT) {
                        // Terminate thread after max idle time
                        --idle_threads;
                        --total_threads;
                        return nullptr;
                    }
                }
            }
            if (pending_task) {
                local_task.swap(pending_task);
                pthread_cond_signal(&recv_task);
            }
            --idle_threads;
        }
        if (local_task)
            local_task();
        if (getpid() == gettid())
            exit(0);
    }
}

void init_thread_pool() {
    pthread_atfork(nullptr, nullptr, &reset_pool);
}

void exec_task(function<void()> &&task) {
    mutex_guard g(lock);
    pending_task.swap(task);
    if (idle_threads == 0) {
        ++total_threads;
        long is_core_pool = total_threads <= CORE_POOL_SIZE;
        new_daemon_thread(thread_pool_loop, (void *) is_core_pool);
    } else {
        pthread_cond_signal(&send_task);
    }
    pthread_cond_wait(&recv_task, &lock);
}
