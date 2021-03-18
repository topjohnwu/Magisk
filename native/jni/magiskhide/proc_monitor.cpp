#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ptrace.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <vector>
#include <bitset>

#include <utils.hpp>

#include "magiskhide.hpp"

using namespace std;

static int inotify_fd = -1;

static void new_zygote(int pid);

pthread_t monitor_thread;

/******************
 * Data structures
 ******************/

#define PID_MAX 32768
struct pid_set {
    bitset<PID_MAX>::const_reference operator[](size_t pos) const { return set[pos - 1]; }
    bitset<PID_MAX>::reference operator[](size_t pos) { return set[pos - 1]; }
    void reset() { set.reset(); }
private:
    bitset<PID_MAX> set;
};

// true if pid is monitored
static pid_set attaches;

// zygote pid -> mnt ns
static map<int, struct stat> zygote_map;

/********
 * Utils
 ********/

static inline int read_ns(const int pid, struct stat *st) {
    char path[32];
    sprintf(path, "/proc/%d/ns/mnt", pid);
    return stat(path, st);
}

static int parse_ppid(int pid) {
    char path[32];
    int ppid;

    sprintf(path, "/proc/%d/stat", pid);

    auto stat = open_file(path, "re");
    if (!stat)
        return -1;

    // PID COMM STATE PPID .....
    fscanf(stat.get(), "%*d %*s %*c %d", &ppid);

    return ppid;
}

static bool is_zygote_done() {
#ifdef __LP64__
    return zygote_map.size() >= 2;
#else
    return zygote_map.size() >= 1;
#endif
}

static void check_zygote() {
    crawl_procfs([](int pid) -> bool {
        char buf[512];
        snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);
        if (FILE *f = fopen(buf, "re")) {
            fgets(buf, sizeof(buf), f);
            if (strncmp(buf, "zygote", 6) == 0 && parse_ppid(pid) == 1)
                new_zygote(pid);
            fclose(f);
        }
        return true;
    });
    if (is_zygote_done()) {
        // Stop periodic scanning
        timeval val { .tv_sec = 0, .tv_usec = 0 };
        itimerval interval { .it_interval = val, .it_value = val };
        setitimer(ITIMER_REAL, &interval, nullptr);
    }
}

#define APP_PROC "/system/bin/app_process"

static void setup_inotify() {
    inotify_fd = xinotify_init1(IN_CLOEXEC);
    if (inotify_fd < 0)
        return;

    // Setup inotify asynchronous I/O
    fcntl(inotify_fd, F_SETFL, O_ASYNC);
    struct f_owner_ex ex = {
        .type = F_OWNER_TID,
        .pid = gettid()
    };
    fcntl(inotify_fd, F_SETOWN_EX, &ex);

    // Monitor packages.xml
    inotify_add_watch(inotify_fd, "/data/system", IN_CLOSE_WRITE);

    // Monitor app_process
    if (access(APP_PROC "32", F_OK) == 0) {
        inotify_add_watch(inotify_fd, APP_PROC "32", IN_ACCESS);
        if (access(APP_PROC "64", F_OK) == 0)
            inotify_add_watch(inotify_fd, APP_PROC "64", IN_ACCESS);
    } else {
        inotify_add_watch(inotify_fd, APP_PROC, IN_ACCESS);
    }
}

/************************
 * Async signal handlers
 ************************/

static void inotify_event(int) {
    // Make sure we can actually read stuffs
    // or else the whole thread will be blocked.
    struct pollfd pfd = {
        .fd = inotify_fd,
        .events = POLLIN,
        .revents = 0
    };
    if (poll(&pfd, 1, 0) <= 0)
        return;  // Nothing to read
    char buf[512];
    auto event = reinterpret_cast<struct inotify_event *>(buf);
    read(inotify_fd, buf, sizeof(buf));
    if ((event->mask & IN_CLOSE_WRITE) && event->name == "packages.xml"sv)
        update_uid_map();
    check_zygote();
}

static void term_thread(int) {
    LOGD("proc_monitor: cleaning up\n");
    zygote_map.clear();
    attaches.reset();
    close(inotify_fd);
    inotify_fd = -1;
    // Restore all signal handlers that was set
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, nullptr);
    struct sigaction act{};
    act.sa_handler = SIG_DFL;
    sigaction(SIGTERMTHRD, &act, nullptr);
    sigaction(SIGIO, &act, nullptr);
    sigaction(SIGALRM, &act, nullptr);
    LOGD("proc_monitor: terminate\n");
    pthread_exit(nullptr);
}

/******************
 * Ptrace Madness
 ******************/

// Ptrace is super tricky, preserve all excessive logging in code
// but disable when actually building for usage (you won't want
// your logcat spammed with new thread events from all apps)

//#define PTRACE_LOG(fmt, args...) LOGD("PID=[%d] " fmt, pid, ##args)
#define PTRACE_LOG(...)

static void detach_pid(int pid, int signal = 0) {
    attaches[pid] = false;
    ptrace(PTRACE_DETACH, pid, 0, signal);
    PTRACE_LOG("detach\n");
}

static bool check_pid(int pid) {
    char path[128];
    char cmdline[1024];
    struct stat st;

    sprintf(path, "/proc/%d", pid);
    if (stat(path, &st)) {
        // Process died unexpectedly, ignore
        detach_pid(pid);
        return true;
    }

    int uid = st.st_uid;

    // UID hasn't changed
    if (uid == 0)
        return false;

    sprintf(path, "/proc/%d/cmdline", pid);
    if (auto f = open_file(path, "re")) {
        fgets(cmdline, sizeof(cmdline), f.get());
    } else {
        // Process died unexpectedly, ignore
        detach_pid(pid);
        return true;
    }

    if (cmdline == "zygote"sv || cmdline == "zygote32"sv || cmdline == "zygote64"sv ||
        cmdline == "usap32"sv || cmdline == "usap64"sv)
        return false;

    if (!is_hide_target(uid, cmdline, 95))
        goto not_target;

    // Ensure ns is separated
    read_ns(pid, &st);
    for (auto &zit : zygote_map) {
        if (zit.second.st_ino == st.st_ino &&
            zit.second.st_dev == st.st_dev) {
            // ns not separated, abort
            LOGW("proc_monitor: skip [%s] PID=[%d] UID=[%d]\n", cmdline, pid, uid);
            goto not_target;
        }
    }

    // Detach but the process should still remain stopped
    // The hide daemon will resume the process after hiding it
    LOGI("proc_monitor: [%s] PID=[%d] UID=[%d]\n", cmdline, pid, uid);
    detach_pid(pid, SIGSTOP);
    hide_daemon(pid);
    return true;

not_target:
    PTRACE_LOG("[%s] is not our target\n", cmdline);
    detach_pid(pid);
    return true;
}

static bool is_process(int pid) {
    char buf[128];
    char key[32];
    int tgid;
    sprintf(buf, "/proc/%d/status", pid);
    auto fp = open_file(buf, "re");
    // PID is dead
    if (!fp)
        return false;
    while (fgets(buf, sizeof(buf), fp.get())) {
        sscanf(buf, "%s", key);
        if (key == "Tgid:"sv) {
            sscanf(buf, "%*s %d", &tgid);
            return tgid == pid;
        }
    }
    return false;
}

static void new_zygote(int pid) {
    struct stat st;
    if (read_ns(pid, &st))
        return;

    auto it = zygote_map.find(pid);
    if (it != zygote_map.end()) {
        // Update namespace info
        it->second = st;
        return;
    }

    LOGD("proc_monitor: ptrace zygote PID=[%d]\n", pid);
    zygote_map[pid] = st;

    xptrace(PTRACE_ATTACH, pid);

    waitpid(pid, nullptr, __WALL | __WNOTHREAD);
    xptrace(PTRACE_SETOPTIONS, pid, nullptr,
            PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACEEXIT);
    xptrace(PTRACE_CONT, pid);
}

#define DETACH_AND_CONT { detach_pid(pid); continue; }

void proc_monitor() {
    monitor_thread = pthread_self();

    // Backup original mask
    sigset_t orig_mask;
    pthread_sigmask(SIG_SETMASK, nullptr, &orig_mask);

    sigset_t unblock_set;
    sigemptyset(&unblock_set);
    sigaddset(&unblock_set, SIGTERMTHRD);
    sigaddset(&unblock_set, SIGIO);
    sigaddset(&unblock_set, SIGALRM);

    struct sigaction act{};
    sigfillset(&act.sa_mask);
    act.sa_handler = SIG_IGN;
    sigaction(SIGTERMTHRD, &act, nullptr);
    sigaction(SIGIO, &act, nullptr);
    sigaction(SIGALRM, &act, nullptr);

    // Temporary unblock to clear pending signals
    pthread_sigmask(SIG_UNBLOCK, &unblock_set, nullptr);
    pthread_sigmask(SIG_SETMASK, &orig_mask, nullptr);

    act.sa_handler = term_thread;
    sigaction(SIGTERMTHRD, &act, nullptr);
    act.sa_handler = inotify_event;
    sigaction(SIGIO, &act, nullptr);
    act.sa_handler = [](int){ check_zygote(); };
    sigaction(SIGALRM, &act, nullptr);

    setup_inotify();

    // First try find existing zygotes
    check_zygote();
    if (!is_zygote_done()) {
        // Periodic scan every 250ms
        timeval val { .tv_sec = 0, .tv_usec = 250000 };
        itimerval interval { .it_interval = val, .it_value = val };
        setitimer(ITIMER_REAL, &interval, nullptr);
    }

    for (int status;;) {
        pthread_sigmask(SIG_UNBLOCK, &unblock_set, nullptr);

        const int pid = waitpid(-1, &status, __WALL | __WNOTHREAD);
        if (pid < 0) {
            if (errno == ECHILD) {
                // Nothing to wait yet, sleep and wait till signal interruption
                LOGD("proc_monitor: nothing to monitor, wait for signal\n");
                struct timespec ts = {
                    .tv_sec = INT_MAX,
                    .tv_nsec = 0
                };
                nanosleep(&ts, nullptr);
            }
            continue;
        }

        pthread_sigmask(SIG_SETMASK, &orig_mask, nullptr);

        if (!WIFSTOPPED(status) /* Ignore if not ptrace-stop */)
            DETACH_AND_CONT;

        int event = WEVENT(status);
        int signal = WSTOPSIG(status);

        if (signal == SIGTRAP && event) {
            unsigned long msg;
            xptrace(PTRACE_GETEVENTMSG, pid, nullptr, &msg);
            if (zygote_map.count(pid)) {
                // Zygote event
                switch (event) {
                    case PTRACE_EVENT_FORK:
                    case PTRACE_EVENT_VFORK:
                        PTRACE_LOG("zygote forked: [%lu]\n", msg);
                        attaches[msg] = true;
                        break;
                    case PTRACE_EVENT_EXIT:
                        PTRACE_LOG("zygote exited with status: [%lu]\n", msg);
                        [[fallthrough]];
                    default:
                        zygote_map.erase(pid);
                        DETACH_AND_CONT;
                }
            } else {
                switch (event) {
                    case PTRACE_EVENT_CLONE:
                        PTRACE_LOG("create new threads: [%lu]\n", msg);
                        if (attaches[pid] && check_pid(pid))
                            continue;
                        break;
                    case PTRACE_EVENT_EXEC:
                    case PTRACE_EVENT_EXIT:
                        PTRACE_LOG("exit or execve\n");
                        [[fallthrough]];
                    default:
                        DETACH_AND_CONT;
                }
            }
            xptrace(PTRACE_CONT, pid);
        } else if (signal == SIGSTOP) {
            if (!attaches[pid]) {
                // Double check if this is actually a process
                attaches[pid] = is_process(pid);
            }
            if (attaches[pid]) {
                // This is a process, continue monitoring
                PTRACE_LOG("SIGSTOP from child\n");
                xptrace(PTRACE_SETOPTIONS, pid, nullptr,
                        PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT);
                xptrace(PTRACE_CONT, pid);
            } else {
                // This is a thread, do NOT monitor
                PTRACE_LOG("SIGSTOP from thread\n");
                DETACH_AND_CONT;
            }
        } else {
            // Not caused by us, resend signal
            xptrace(PTRACE_CONT, pid, nullptr, signal);
            PTRACE_LOG("signal [%d]\n", signal);
        }
    }
}
