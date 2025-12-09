use base::{ResultExt, new_daemon_thread};
use nix::sys::signal::SigSet;
use nix::unistd::{getpid, gettid};
use std::sync::nonpoison::{Condvar, Mutex};
use std::sync::{LazyLock, WaitTimeoutResult};
use std::time::Duration;

static THREAD_POOL: LazyLock<ThreadPool> = LazyLock::new(ThreadPool::default);

const THREAD_IDLE_MAX_SEC: u64 = 60;
const CORE_POOL_SIZE: i32 = 3;

#[derive(Default)]
pub struct ThreadPool {
    task_is_some: Condvar,
    task_is_none: Condvar,
    info: Mutex<PoolInfo>,
}

#[derive(Default)]
struct PoolInfo {
    idle_threads: i32,
    total_threads: i32,
    task: Option<Box<dyn FnOnce() + Send>>,
}

impl ThreadPool {
    fn pool_loop(&self, is_core_pool: bool) {
        let mask = SigSet::all();

        loop {
            // Always restore the sigmask to block all signals
            mask.thread_set_mask().log_ok();

            let task: Option<Box<dyn FnOnce() + Send>>;
            {
                let mut info = self.info.lock();
                info.idle_threads += 1;
                if info.task.is_none() {
                    if is_core_pool {
                        // Core pool never closes, wait forever.
                        info = self.task_is_some.wait(info);
                    } else {
                        let dur = Duration::from_secs(THREAD_IDLE_MAX_SEC);
                        let result: WaitTimeoutResult;
                        (info, result) = self.task_is_some.wait_timeout(info, dur);
                        if result.timed_out() {
                            // Terminate thread after timeout
                            info.idle_threads -= 1;
                            info.total_threads -= 1;
                            return;
                        }
                    }
                }
                task = info.task.take();
                self.task_is_none.notify_one();
                info.idle_threads -= 1;
            }
            if let Some(task) = task {
                task();
            }
            if getpid() == gettid() {
                // This meant the current thread forked and became the main thread, exit
                std::process::exit(0);
            }
        }
    }

    fn exec_task_impl(&self, f: impl FnOnce() + Send + 'static) {
        extern "C" fn pool_loop_raw(arg: usize) -> usize {
            let is_core_pool = arg != 0;
            THREAD_POOL.pool_loop(is_core_pool);
            0
        }

        let mut info = self.info.lock();
        while info.task.is_some() {
            // Wait until task is none
            info = self.task_is_none.wait(info);
        }
        info.task = Some(Box::new(f));
        if info.idle_threads == 0 {
            info.total_threads += 1;
            let is_core_thread = if info.total_threads <= CORE_POOL_SIZE {
                1_usize
            } else {
                0_usize
            };
            unsafe {
                new_daemon_thread(pool_loop_raw, is_core_thread);
            }
        } else {
            self.task_is_some.notify_one();
        }
    }

    pub fn exec_task(f: impl FnOnce() + Send + 'static) {
        THREAD_POOL.exec_task_impl(f);
    }
}
