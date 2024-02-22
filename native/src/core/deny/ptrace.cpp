#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <bitset>
#include <string>
#include <cinttypes>
#include <poll.h>
#include <sys/syscall.h>
#include <sys/inotify.h>
#include <errno.h>

#include <core.hpp>
#include <consts.hpp>
#include <base.hpp>
#include <selinux.hpp>

#include "deny.hpp"
#include <sys/ptrace.h>

using namespace std;

static long xptrace(int request, pid_t pid, void *addr = nullptr, void *data = nullptr) {
    long ret = ptrace(request, pid, addr, data);
    if (ret < 0)
        PLOGE("ptrace %d", pid);
    return ret;
}

static inline long xptrace(int request, pid_t pid, void *addr, uintptr_t data) {
    return xptrace(request, pid, addr, reinterpret_cast<void *>(data));
}

#define WEVENT(s) (((s) & 0xffff0000) >> 16)

// Process monitoring
pthread_t monitor_thread;
static int inotify_fd = -1;
static int data_system_wd = -1;

static int parse_ppid(int pid);
static bool check_process(int pid, const char *process = 0, const char *context = 0, const char *exe = 0);
static bool is_process(int pid, int uid = 0);
static void new_zygote(int pid);

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

// zygote pid -> mnt ns
static map<int, struct stat> zygote_map;

// attaches set
static pid_set attaches;

// other set
static pid_set allowed;
static pid_set checked;

/********
 * Utils
 ********/

// #define PTRACE_LOG(fmt, args...) LOGD("PID=[%d] " fmt, pid, ##args)
#define PTRACE_LOG(...)

static void detach_pid(int pid, int signal = 0) {
    attaches[pid] = false;
    allowed[pid] = false;
    checked[pid] = false;
    ptrace(PTRACE_DETACH, pid, 0, signal);
    PTRACE_LOG("detach\n");
}

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
    int zygote_count = (HAVE_32)? 2:1;
    if (zygote_map.size() >= zygote_count)
        return true;
#else
    if (zygote_map.size() >= 1)
        return true;
#endif

    return false;
}

static inline bool read_file(const char *file, char *buf, int count){
    FILE *fp = fopen(file, "re");
    if (!fp) return false;
    fread(buf, count, 1, fp);
    fclose(fp);
    return true;
}

static bool check_process(int pid, const char *process, const char *context, const char *exe) {
    char path[128];
    char buf[1024];
    ssize_t len;

    if (!process) goto check_context;
    sprintf(path, "/proc/%d/cmdline", pid);
    if (!read_file(path,buf,sizeof(buf)) ||
        strcmp(buf, process) != 0)
        return false;

    check_context:
    if (!context) goto check_exe;
    sprintf(path, "/proc/%d/attr/current", pid);
    if (!read_file(path,buf,sizeof(buf)) || 
        !str_contains(buf, context))
        return false;

    check_exe:
    if (!exe) goto final;
    sprintf(path, "/proc/%d/exe", pid);
    len = readlink(path, buf, sizeof(buf)-1);
    if (len != -1) {
      buf[len] = '\0';
    }
    if (strcmp(buf, exe) != 0)
        return false;

    final:
    return true;
}

static bool is_zygote(int pid){
    return check_process(pid, "zygote", "u:r:zygote:s0", nullptr)  
        || check_process(pid, "zygote64", "u:r:zygote:s0", nullptr)
        || check_process(pid, "zygote32", "u:r:zygote:s0", nullptr);
}

static void check_zygote(){
    bool system_server_started = false;
    vector<int> zygote_list;

    crawl_procfs([&zygote_list, &system_server_started](int pid) -> bool {
        // Zygote process
        if (is_process(pid) && is_zygote(pid) && parse_ppid(pid) == 1) {
            zygote_list.push_back(pid);
            return true;
        }

        // system_server: pid == 1000 and zygote is ppid
        if (is_process(pid, 1000) && is_zygote(parse_ppid(pid))) {
            system_server_started = true;
            return true;
        }

        // Others
        return true;
    });

    if (system_server_started) {
        // system_server, starting trace zygote
        for (int i = 0; i < zygote_list.size(); i++) {
            new_zygote(zygote_list[i]);
        }
    }

    if (is_zygote_done()) {
        // Stop periodic scanning
        timeval val { .tv_sec = 0, .tv_usec = 0 };
        itimerval interval { .it_interval = val, .it_value = val };
        setitimer(ITIMER_REAL, &interval, nullptr);
    }
}

void umount_all_zygote() {
    crawl_procfs([](int pid) -> bool {
        // Unmount all Magisk from zygote process by default
        if (is_process(pid) && is_zygote(pid) && parse_ppid(pid) == 1) {
            revert_daemon(pid, -2);
        }
        return true;
    });
}       

#define APP_PROC "/system/bin/app_process"

static void setup_inotify() {
    inotify_fd = inotify_init1(IN_CLOEXEC);
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
    data_system_wd = inotify_add_watch(inotify_fd, "/data/system", IN_CLOSE_WRITE);

    // Monitor app installation
    inotify_add_watch(inotify_fd, APP_DATA_DIR, IN_CREATE);
    DIR *dirfp = opendir(APP_DATA_DIR);
    if (dirfp) {
           char buf[4098];
        struct dirent *dp;
        while ((dp = readdir(dirfp)) != nullptr) {
            ssprintf(buf, sizeof(buf) - 1, "%s/%s", APP_DATA_DIR, dp->d_name);
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
                continue;
            LOGD("proc_monitor: monitor userspace ID=[%s]\n", dp->d_name);
            inotify_add_watch(inotify_fd, buf, IN_ATTRIB);
        }
        closedir(dirfp);
    }

    // Monitor app_process
    if (access(APP_PROC "32", F_OK) == 0) {
        inotify_add_watch(inotify_fd, APP_PROC "32", IN_ACCESS);
        if (access(APP_PROC "64", F_OK) == 0)
            inotify_add_watch(inotify_fd, APP_PROC "64", IN_ACCESS);
    } else {
        inotify_add_watch(inotify_fd, APP_PROC, IN_ACCESS);
    }
}

static bool is_process(int pid, int uid) {
    char buf[128];
    char key[32];
    int tgid;
    struct stat st{};
    sprintf(buf, "/proc/%d", pid);
    if (stat(buf, &st) || st.st_uid != uid)
        return false;
    sprintf(buf, "/proc/%d/status", pid);
    auto fp = fopen(buf, "re");
    // PID is dead
    if (!fp)
        return false;
    while (fgets(buf, sizeof(buf), fp)) {
        sscanf(buf, "%s", key);
        if (key == "Tgid:"sv) {
            sscanf(buf, "%*s %d", &tgid);
            fclose(fp);
            return tgid == pid;
        }
    }
    fclose(fp);
    return false;
}

/************************
 * Async signal handlers
 ************************/

#define USAP_ENABLED "persist.device_config.runtime_native.usap_pool_enabled" 

// Make sure we can actually read stuffs 
// or else the whole thread will be blocked. 
#define POLL_EVENT \
    struct pollfd pfd = { \
        .fd = inotify_fd, \
        .events = POLLIN, \
        .revents = 0 \
    }; \
    if (poll(&pfd, 1, 0) <= 0) \
        return;

#define PROCESS_EVENT \
    do { \
        char buf[512]; \
        auto event = reinterpret_cast<struct inotify_event *>(buf); \
        read(inotify_fd, buf, sizeof(buf)); \
        if (event->mask & IN_CREATE) { \
            std::string path = std::string(APP_DATA_DIR) + "/" + event->name; \
            LOGD("proc_monitor: monitor userspace ID=[%s]\n", event->name); \
            inotify_add_watch(inotify_fd, path.data(), IN_ATTRIB); \
            break; \
        } \
        if ((event->wd == data_system_wd && event->name == "packages.xml"sv) || (event->mask & IN_ATTRIB)) \
            rescan_apps(); \
        check_zygote(); \
    } while (false);



static void inotify_event(int) {
    POLL_EVENT
    PROCESS_EVENT
}

static void term_thread(int) {
    LOGD("proc_monitor: cleaning up\n");
    zygote_map.clear();
    attaches.reset();
    checked.reset();
    allowed.reset();
    close(inotify_fd);
    inotify_fd = -1;
    monitor_thread = -1;
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

static bool ino_equal(struct stat st, struct stat st2){
    return st.st_dev == st2.st_dev &&
        st.st_ino == st2.st_ino;
}

static int check_pid(int pid) {
    char path[128];
    char cmdline[1024];
    int uid = -1;
    int ppid = -1;
    struct stat st;
    sprintf(path, "/proc/%d", pid);
    if (stat(path, &st)) {
        // Process died unexpectedly, ignore
        goto not_target;
    }
    uid = st.st_uid;
    if (uid == 0) {
        return 0;
    }

    // check cmdline
    ssprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    if (!read_file(path, cmdline, sizeof(cmdline)))
        // Process died unexpectedly, ignore
        goto not_target;

    // still zygote
    if (cmdline == "zygote"sv || cmdline == "zygote32"sv || cmdline == "zygote64"sv ||
        cmdline == "usap32"sv || cmdline == "usap64"sv || cmdline == "<pre-initialized>"sv)
        return 0;

    if (!is_deny_target(uid, cmdline, 95)) {
        goto not_target;
    }

    // Ensure ns is separated
    {
        struct stat ppid_st;
        ppid = parse_ppid(pid);
        read_ns(pid, &st);
        read_ns(ppid, &ppid_st);
        if (ino_equal(st, ppid_st)) {
            LOGW("proc_monitor: skip [%s] PID=[%d] PPID=[%d] UID=[%d]\n", cmdline, pid, ppid, uid);
            goto not_target;
        }
    }

    LOGI("proc_monitor: [%s] PID=[%d] UID=[%d]\n", cmdline, pid, uid);
    detach_pid(pid);
    kill(pid, SIGSTOP);

    {
        (sulist_enabled) ?
        // if sulist is enabled
        // the target is the process we want to mount magisk
        // else, the target is the process we want to unmount magisk
        mount_magisk_to_pid(pid) : revert_daemon(pid);
    }

not_target:
    detach_pid(pid);
    return 1;
}

static void new_zygote(int pid) {
    struct stat st, init_st;
    if (read_ns(pid, &st) || read_ns(1, &init_st) || 
        (init_st.st_ino == st.st_ino && init_st.st_dev == st.st_dev))
        return;

    auto it = zygote_map.find(pid);
    if (it != zygote_map.end()) {
        it->second = st;
        return;
    }

    // check if pid is attached
    if (zygote_map.count(pid))
        return;

    LOGI("proc_monitor: zygote PID=[%d]\n", pid);

    // attach_zygote
    if (xptrace(PTRACE_ATTACH, pid) == -1)
        return;
    LOGI("proc_monitor: ptrace zygote PID=[%d]\n", pid);
    zygote_map[pid] = st;

    if (sulist_enabled) revert_daemon(pid, -2);

    waitpid(pid, nullptr, __WALL | __WNOTHREAD);
    xptrace(PTRACE_SETOPTIONS, pid, nullptr,
            PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACEEXIT);
    xptrace(PTRACE_CONT, pid);
}

#define DETACH_AND_CONT { detach_pid(pid); continue; }

static std::string get_content(int pid, const char *file) {
    char buf[1024];
    sprintf(buf, "/proc/%d/%s", pid, file);
    FILE *fp = fopen(buf, "re");
    if (fp) {
        fgets(buf, sizeof(buf), fp);
        fclose(fp);
        return std::string(buf);
    }
    return std::string("");
}

void proc_monitor() {
    monitor_thread = pthread_self();

    // Reset cached result
    zygote_map.clear();
    attaches.reset();
    checked.reset();
    allowed.reset();

    // Backup original mask
    sigset_t orig_mask;
    pthread_sigmask(SIG_SETMASK, nullptr, & orig_mask);

    sigset_t unblock_set;
    sigemptyset( & unblock_set);
    sigaddset( & unblock_set, SIGTERMTHRD);
    sigaddset( & unblock_set, SIGIO);
    sigaddset( & unblock_set, SIGALRM);

    struct sigaction act {};
    sigfillset( & act.sa_mask);
    act.sa_handler = SIG_IGN;
    sigaction(SIGTERMTHRD, & act, nullptr);
    sigaction(SIGIO, & act, nullptr);
    sigaction(SIGALRM, & act, nullptr);

    // Temporary unblock to clear pending signals
    pthread_sigmask(SIG_UNBLOCK, & unblock_set, nullptr);
    pthread_sigmask(SIG_SETMASK, & orig_mask, nullptr);

    act.sa_handler = term_thread;
    sigaction(SIGTERMTHRD, & act, nullptr);
    act.sa_handler = inotify_event;
    sigaction(SIGIO, & act, nullptr);
    act.sa_handler = [](int) {
        check_zygote();
    };
    sigaction(SIGALRM, & act, nullptr);

    setup_inotify();

    // First try find existing system server and zygote
    check_zygote();
    rescan_apps();
    if (!is_zygote_done()) {
        // Periodic scan every 250ms
        timeval val {
            .tv_sec = 0, .tv_usec = 250000
        };
        itimerval interval {
            .it_interval = val, .it_value = val
        };
        setitimer(ITIMER_REAL, & interval, nullptr);
    }

    for (int status;;) {
        pthread_sigmask(SIG_UNBLOCK, & unblock_set, nullptr);
        const int pid = waitpid(-1, & status, __WALL | __WNOTHREAD);
        if (pid < 0) {
            if (errno == ECHILD) {
                // Nothing to wait yet, sleep and wait till signal interruption
                LOGD("proc_monitor: nothing to monitor, wait for signal\n");
                struct timespec ts = {
                    .tv_sec = INT_MAX,
                    .tv_nsec = 0
                };
                nanosleep( & ts, nullptr);
            }
            continue;
        }

        pthread_sigmask(SIG_SETMASK, & orig_mask, nullptr);

        if (!WIFSTOPPED(status) /* Ignore if not ptrace-stop */ )
            DETACH_AND_CONT;

        int event = WEVENT(status);
        int signal = WSTOPSIG(status);

        if (signal == SIGTRAP && zygote_map.count(pid) & event) {
            unsigned long msg;
            xptrace(PTRACE_GETEVENTMSG, pid, nullptr, & msg);
            switch (event) {
            case PTRACE_EVENT_FORK:
            case PTRACE_EVENT_VFORK:
                PTRACE_LOG("zygote forked: [%lu]\n", msg);
                attaches[msg] = true;
                break;
            case PTRACE_EVENT_EXIT:
                PTRACE_LOG("zygote exited with status: [%lu]\n", msg);
                [
                    [fallthrough]
                ];
            default:
                zygote_map.erase(pid);
                DETACH_AND_CONT;
            }
            xptrace(PTRACE_CONT, pid);
        } else if (signal == (SIGTRAP | 0x80)) {
            do {
                struct stat st {};
                char path[128];
                if (checked[pid]) goto CHECK_PROC;
                sprintf(path, "/proc/%d", pid);
                stat(path, & st);
                PTRACE_LOG("UID=[%d]\n", st.st_uid);
                if (st.st_uid == 0)
                    continue;
                //LOGD("proc_monitor: PID=[%d] UID=[%d]\n", pid, st.st_uid);
                if ((st.st_uid % 100000) >= 90000) {
                    PTRACE_LOG("is isolated process\n");
                    if (sulist_enabled)
                        goto DETACH_PROC;
                    goto CHECK_PROC;
                }

                // check if UID is on list
                if (!is_uid_on_list(st.st_uid))
                    goto DETACH_PROC;

                CHECK_PROC:
                    checked[pid] = true;
                if (!allowed[pid] && (
                        // app zygote
                        strstr(get_content(pid, "attr/current").data(), "u:r:app_zygote:s0") ||
                        // until pre-initialized
                        get_content(pid, "cmdline") == "<pre-initialized>"))
                    allowed[pid] = true;

                if (!allowed[pid])
                    continue;

                if (check_pid(pid))
                    goto skip;
                continue;

                DETACH_PROC:
                    detach_pid(pid);
                goto skip;
            } while (false);
            xptrace(PTRACE_SYSCALL, pid);
        } else if (signal == SIGSTOP) {
            // SIGSTOP is produced by ptrace
            if (!attaches[pid]) {
                // Double check if this is actually a process
                attaches[pid] = is_process(pid);
            }
            if (attaches[pid]) {
                // This is a process, continue monitoring
                PTRACE_LOG("SIGSTOP from child\n");
                xptrace(PTRACE_SETOPTIONS, pid, nullptr,
                    PTRACE_O_TRACESYSGOOD);
                xptrace(PTRACE_SYSCALL, pid);
                // TODO : inject syscall
            } else {
                // This is a thread, do NOT monitor
                PTRACE_LOG("SIGSTOP from thread\n");
                DETACH_AND_CONT;
            }
        } else {
            // Not caused by us, resend signal
            xptrace((!zygote_map.count(pid) && attaches[pid]) ? 
                    PTRACE_SYSCALL : PTRACE_CONT, pid, nullptr, signal);
            PTRACE_LOG("signal [%d]\n", signal);
        }

        skip:
            continue;
    }
}
