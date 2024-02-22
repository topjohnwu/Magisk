// This function is based on the code snippet from https://github.com/5ec1cff/ZygiskNext/blob/c205893dd2f4cfac0ddc77d78cac4f6dab90195a/loader/src/ptracer/utils.hpp
// Original author: 5ec1cff

#pragma once
#include <string>
#include <sys/ptrace.h>
#include <lsplt.hpp>

#if defined(__x86_64__)
#define REG_SP rsp
#define REG_IP rip
#define REG_RET rax
#elif defined(__i386__)
#define REG_SP esp
#define REG_IP eip
#define REG_RET eax
#elif defined(__aarch64__)
#define REG_SP sp
#define REG_IP pc
#define REG_RET regs[0]
#elif defined(__arm__)
#define REG_SP uregs[13]
#define REG_IP uregs[15]
#define REG_RET uregs[0]
#define user_regs_struct user_regs
#endif

#define WPTEVENT(x) (x >> 16)

std::vector<lsplt::MapInfo> Scan_proc(const std::string& pid = "self");

ssize_t write_proc(int pid, uintptr_t *remote_addr, const void *buf, size_t len);

ssize_t read_proc(int pid, uintptr_t *remote_addr, void *buf, size_t len);

bool get_regs(int pid, struct user_regs_struct &regs);

bool set_regs(int pid, struct user_regs_struct &regs);

std::string get_addr_mem_region(std::vector<lsplt::MapInfo> &info, void *addr);

void *find_module_return_addr(std::vector<lsplt::MapInfo> &info, std::string_view suffix);

void *find_module_base(std::vector<lsplt::MapInfo> &info, std::string_view suffix);

void *find_func_addr(
        std::vector<lsplt::MapInfo> &local_info,
        std::vector<lsplt::MapInfo> &remote_info,
        std::string_view module,
        std::string_view func);

void align_stack(struct user_regs_struct &regs, long preserve = 0);

void *push_string(int pid, struct user_regs_struct &regs, const char *str);

uintptr_t remote_call(int pid, struct user_regs_struct &regs, uintptr_t func_addr, uintptr_t return_addr,
                 std::vector<long> &args);

void wait_for_trace(int pid, int* status, int flags);

std::string parse_status(int status);

#define CASE_CONST_RETURN(x) case x: return #x;

inline const char* parse_ptrace_event(int status) {
    status = status >> 16;
    switch (status) {
        CASE_CONST_RETURN(PTRACE_EVENT_FORK)
        CASE_CONST_RETURN(PTRACE_EVENT_VFORK)
        CASE_CONST_RETURN(PTRACE_EVENT_CLONE)
        CASE_CONST_RETURN(PTRACE_EVENT_EXEC)
        CASE_CONST_RETURN(PTRACE_EVENT_VFORK_DONE)
        CASE_CONST_RETURN(PTRACE_EVENT_EXIT)
        CASE_CONST_RETURN(PTRACE_EVENT_SECCOMP)
        CASE_CONST_RETURN(PTRACE_EVENT_STOP)
        default:
            return "(no event)";
    }
}

inline const char* sigabbrev_np(int sig) {
    if (sig > 0 && sig < NSIG) return sys_signame[sig];
    return "(unknown)";
}
