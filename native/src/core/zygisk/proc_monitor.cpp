// This function is based on the code snippet from https://github.com/5ec1cff/ZygiskNext/blob/c205893dd2f4cfac0ddc77d78cac4f6dab90195a/loader/src/ptracer/monitor.cpp
// Original author: 5ec1cff

#include <base.hpp>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <string>
#include <set>
#include <sys/stat.h>

#include <base.hpp>
#include <core.hpp>
#include <consts.hpp>

#include "zygisk.hpp"
#include "ptrace_utils.hpp"

using namespace std;

#define WEVENT(__status) (((__status) >> 16) & 0xff)

#define STOPPED_WITH(sig, event) WIFSTOPPED(status) && (status >> 8 == ((sig) | (event << 8)))

// #define PTRACE_LOG(fmt, args...) LOGD(fmt, ##args)
#define PTRACE_LOG(...)

std::string get_program(int pid) {
    std::string path = "/proc/";
    path += std::to_string(pid);
    path += "/exe";
    constexpr const auto SIZE = 256;
    char buf[SIZE + 1];
    auto sz = readlink(path.c_str(), buf, SIZE);
    if (sz == -1) {
        PLOGE("readlink /proc/%d/exe", pid);
        return "";
    }
    buf[sz] = 0;
    return buf;
}

static inline int read_ns(const int pid, struct stat *st) {
    char path[32];
    sprintf(path, "/proc/%d/ns/mnt", pid);
    return stat(path, st);
}

static bool unmount_zygote(int pid, int fd) {
    struct stat st, init_st;
    if (access(("/proc/self/fd/"s + to_string(fd) + "/cmdline").data(), F_OK) != 0 || read_ns(pid, &st))
        return true;

    if (read_ns(1, &init_st) || 
        (init_st.st_ino == st.st_ino && init_st.st_dev == st.st_dev))
        return false;

    if (access(("/proc/self/fd/"s + to_string(system_server_fd) + "/cmdline").data(), F_OK) != 0)
        return false;

    revert_daemon(pid, -2);
    return true;
}

static int last_pid = 0;

static void *wait_unmount() {
    int pid = last_pid;
    int fd = xopen(("/proc/"s + to_string(pid)).data(), O_PATH);
    while (!unmount_zygote(pid, fd))
        usleep(1000000);
    close(fd);
    return nullptr;
}

void * init_monitor() {
    int status;
    std::set < pid_t > process;

    if (ptrace(PTRACE_SEIZE, 1, 0, PTRACE_O_TRACEFORK) == -1) {
        LOGE("proc_monitor: cannot ptrace init\n");
        goto abandon;
    }
    LOGI("proc_monitor: start tracing init\n");

    for (int pid;;) {
        while ((pid = waitpid(-1, & status, __WALL | __WNOTHREAD)) != 0) {
            if (pid < 0) // If any error occurs, give up
                goto abandon;
            if (pid == 1) {
                if (STOPPED_WITH(SIGTRAP, PTRACE_EVENT_FORK)) {
                    long child_pid;
                    ptrace(PTRACE_GETEVENTMSG, pid, 0, & child_pid);
                    PTRACE_LOG("proc_monitor: init forked %ld\n", child_pid);
                }
                if (WIFSTOPPED(status)) {
                    ptrace(PTRACE_CONT, pid, 0, (WPTEVENT(status) == 0)? WSTOPSIG(status) : 0);
                }
                continue;
            }
            auto state = process.find(pid);
            if (state == process.end()) {
                PTRACE_LOG("proc_monitor: attached pid=%d\n", pid);
                process.emplace(pid);
                ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACEEXEC);
                ptrace(PTRACE_CONT, pid, 0, 0);
                continue;
            } else {
                if (STOPPED_WITH(SIGTRAP, PTRACE_EVENT_EXEC)) {
                    auto program = get_program(pid);
                    LOGD("proc_monitor: pid=[%d] [%s]\n", pid, program.c_str());
                    string tracer = "";
                    do {
                        if (program == "/system/bin/app_process64") {
                            tracer = string(get_magisk_tmp()) + "/magisk64";
                        } else if (program == "/system/bin/app_process32") {
                            tracer = string(get_magisk_tmp()) + "/magisk32";
                        }
                        if (tracer != "" && !stop_trace_zygote) {
                            if (sulist_enabled) {
                                last_pid = pid;
                                new_daemon_thread(reinterpret_cast<thread_entry>(&wait_unmount));
                            }
                            kill(pid, SIGSTOP);
                            ptrace(PTRACE_CONT, pid, 0, 0);
                            waitpid(pid, & status, __WALL);
                            if (STOPPED_WITH(SIGSTOP, 0)) {
                                ptrace(PTRACE_DETACH, pid, 0, SIGSTOP);
                                status = 0;
                                auto p = fork_dont_care();
                                if (p == 0) {
                                    LOGI("zygisk: inject zygote PID=[%d] [%s]\n", pid, program.c_str());
                                    execl(tracer.data(), "", "zygisk", "trace_zygote",
                                        std::to_string(pid).c_str(), tracer.data(), nullptr);
                                    PLOGE("failed to exec");
                                    kill(pid, SIGKILL);
                                    exit(1);
                                } else if (p == -1) {
                                    PLOGE("failed to fork");
                                    kill(pid, SIGKILL);
                                }
                            }
                        }
                    } while (false);

                } else {
                    PTRACE_LOG("process %d received unknown status\n", pid);
                }
                process.erase(state);
                if (WIFSTOPPED(status)) {
                    PTRACE_LOG("proc_monitor: detach process %d\n", pid);
                    ptrace(PTRACE_DETACH, pid, 0, 0);
                }
            }
        }
    }
    abandon:
    LOGI("proc_monitor: stop tracing init\n");
    return nullptr;
}

void start_zygisk() {
    new_daemon_thread(reinterpret_cast<thread_entry>(&init_monitor));
}



