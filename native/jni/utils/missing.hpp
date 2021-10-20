#pragma once

#include <sys/syscall.h>
#include <unistd.h>

static inline int sigtimedwait(const sigset_t* set, siginfo_t* info, const timespec* timeout) {
    union {
        sigset_t set;
        sigset_t set64;
    } s{};
    s.set = *set;
    return syscall(__NR_rt_sigtimedwait, &s.set64, info, timeout, sizeof(sigset64_t));
}
