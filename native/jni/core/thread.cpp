// Cached thread pool implementation

#include <utils.hpp>

#include <daemon.hpp>

using namespace std;

#define THREAD_IDLE_MAX_SEC 60
#define CORE_POOL_SIZE 3

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t send_task = PTHREAD_COND_INITIALIZER;
static pthread_cond_t recv_task = PTHREAD_COND_INITIALIZER;

// The following variables should be guarded by lock
static int available_threads = 0;
static int active_threads = 0;
static function<void()> pending_task;

static void operator+=(timeval &a, const timeval &b) {
    a.tv_sec += b.tv_sec;
    a.tv_usec += b.tv_usec;
    if (a.tv_usec >= 1000000) {
        a.tv_sec++;
        a.tv_usec -= 1000000;
    }
}

static timespec to_ts(const timeval &tv) { return { tv.tv_sec, tv.tv_usec * 1000 };  }

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
            ++available_threads;
            if (!pending_task) {
                if (is_core_pool) {
                    pthread_cond_wait(&send_task, &lock);
                } else {
                    timeval tv;
                    gettimeofday(&tv, nullptr);
                    tv += { THREAD_IDLE_MAX_SEC, 0 };
                    auto ts = to_ts(tv);
                    if (pthread_cond_timedwait(&send_task, &lock, &ts) == ETIMEDOUT) {
                        // Terminate thread after max idle time
                        --available_threads;
                        --active_threads;
                        return nullptr;
                    }
                }
            }
            local_task.swap(pending_task);
            pthread_cond_signal(&recv_task);
            --available_threads;
        }
        local_task();
    }
}

void exec_task(function<void()> &&task) {
    mutex_guard g(lock);
    pending_task.swap(task);
    if (available_threads == 0) {
        ++active_threads;
        long is_core_pool = active_threads <= CORE_POOL_SIZE;
        new_daemon_thread(thread_pool_loop, (void *) is_core_pool);
    } else {
        pthread_cond_signal(&send_task);
    }
    pthread_cond_wait(&recv_task, &lock);
}
