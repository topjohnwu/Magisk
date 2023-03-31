#pragma once

#include <sys/syscall.h>
#include <linux/fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>

static inline int sigtimedwait(const sigset_t* set, siginfo_t* info, const timespec* timeout) {
    union {
        sigset_t set;
        sigset64_t set64;
    } s{};
    s.set = *set;
    return syscall(__NR_rt_sigtimedwait, &s.set64, info, timeout, sizeof(s.set64));
}

static inline int fexecve(int fd, char* const* argv, char* const* envp) {
    syscall(__NR_execveat, fd, "", argv, envp, AT_EMPTY_PATH);
    if (errno == ENOSYS) {
        char buf[256];
        std::snprintf(buf, sizeof(buf), "/proc/self/fd/%d", fd);
        execve(buf, argv, envp);
    }
    return -1;
}
