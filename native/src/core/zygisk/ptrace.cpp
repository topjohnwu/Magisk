// This function is based on the code snippet from https://github.com/5ec1cff/ZygiskNext/blob/c205893dd2f4cfac0ddc77d78cac4f6dab90195a/loader/src/ptracer/ptracer.cpp
// Original author: 5ec1cff

#include <sys/ptrace.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/auxv.h>
#include <elf.h>
#include <link.h>
#include <vector>
#include <string>
#include <sys/mman.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <signal.h>
#include <sys/system_properties.h>
#include <string>

#include <consts.hpp>
#include <base.hpp>
#include <selinux.hpp>

#if defined(__LP64__)
# define LP_SELECT(lp32, lp64) lp64
#else
# define LP_SELECT(lp32, lp64) lp32
#endif

#define ZYGISK_LIB "/dev/magisk_zygisk" LP_SELECT("32", "64") "_"

#include "zygisk.hpp"
#include "ptrace_utils.hpp"

bool inject_on_main(int pid, const char *lib_path) {
    // parsing KernelArgumentBlock
    // https://cs.android.com/android/platform/superproject/main/+/main:bionic/libc/private/KernelArgumentBlock.h;l=30;drc=6d1ee77ee32220e4202c3066f7e1f69572967ad8
    struct user_regs_struct regs{}, backup{};
    auto map = Scan_proc(std::to_string(pid));
    if (!get_regs(pid, regs)) return false;
    auto arg = reinterpret_cast<uintptr_t *>(regs.REG_SP);
    ZLOGD("kernel argument %p %s\n", arg, get_addr_mem_region(map, arg).c_str());
    int argc;
    auto argv = reinterpret_cast<char **>(reinterpret_cast<uintptr_t *>(arg) + 1);
    ZLOGD("argv %p\n", argv);
    read_proc(pid, arg, &argc, sizeof(argc));
    ZLOGD("argc %d\n", argc);
    auto envp = argv + argc + 1;
    ZLOGD("envp %p\n", envp);
    auto p = envp;
    while (true) {
        uintptr_t *buf;
        read_proc(pid, (uintptr_t *) p, &buf, sizeof(buf));
        if (buf != nullptr) ++p;
        else break;
    }
    ++p;
    auto auxv = reinterpret_cast<ElfW(auxv_t) *>(p);
    ZLOGD("auxv %p %s\n", auxv, get_addr_mem_region(map, auxv).c_str());
    auto v = auxv;
    void *entry_addr = nullptr;
    void *addr_of_entry_addr = nullptr;
    while (true) {
        ElfW(auxv_t) buf;
        read_proc(pid, (uintptr_t *) v, &buf, sizeof(buf));
        if (buf.a_type == AT_ENTRY) {
            entry_addr = reinterpret_cast<void *>(buf.a_un.a_val);
            addr_of_entry_addr = reinterpret_cast<char *>(v) + offsetof(ElfW(auxv_t), a_un);
            ZLOGD("entry address %p %s (v=%p, entry_addr=%p)\n", entry_addr,
                 get_addr_mem_region(map, entry_addr).c_str(), v, addr_of_entry_addr);
            break;
        }
        if (buf.a_type == AT_NULL) break;
        v++;
    }
    if (entry_addr == nullptr) {
        ZLOGE("failed to get entry\n");
        return false;
    }

    // Replace the program entry with an invalid address
    // For arm32 compatibility, we set the last bit to the same as the entry address
    uintptr_t break_addr = (-0x05ec1cff & ~1) | ((uintptr_t) entry_addr & 1);
    if (!write_proc(pid, (uintptr_t *) addr_of_entry_addr, &break_addr, sizeof(break_addr))) return false;
    ptrace(PTRACE_CONT, pid, 0, 0);
    int status;
    wait_for_trace(pid, &status, __WALL);
    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSEGV) {
        if (!get_regs(pid, regs)) return false;
        if ((regs.REG_IP & ~1) != (break_addr & ~1)) {
            ZLOGE("stopped at unknown addr %p\n", (void *) regs.REG_IP);
            return false;
        }
        // The linker has been initialized now, we can do dlopen
        ZLOGD("stopped at entry\n");

        // restore entry address
        if (!write_proc(pid, (uintptr_t *) addr_of_entry_addr, &entry_addr, sizeof(entry_addr))) return false;

        // backup registers
        memcpy(&backup, &regs, sizeof(regs));
        map = Scan_proc(std::to_string(pid));
        auto local_map = Scan_proc();
        auto libc_return_addr = find_module_return_addr(map, "libc.so");
        ZLOGD("libc return addr %p\n", libc_return_addr);

        // call dlopen
        auto dlopen_addr = find_func_addr(local_map, map, "libdl.so", "dlopen");
        if (dlopen_addr == nullptr) return false;
        std::vector<long> args;
        auto str = push_string(pid, regs, lib_path);
        args.clear();
        args.push_back((long) str);
        args.push_back((long) RTLD_NOW);
        auto remote_handle = remote_call(pid, regs, (uintptr_t) dlopen_addr, (uintptr_t) libc_return_addr, args);
        ZLOGD("remote handle %p\n", (void *) remote_handle);
        if (remote_handle == 0) {
            ZLOGE("handle is null\n");
            // call dlerror
            auto dlerror_addr = find_func_addr(local_map, map, "libdl.so", "dlerror");
            if (dlerror_addr == nullptr) {
                ZLOGE("find dlerror\n");
                return false;
            }
            args.clear();
            auto dlerror_str_addr = remote_call(pid, regs, (uintptr_t) dlerror_addr, (uintptr_t) libc_return_addr, args);
            ZLOGD("dlerror str %p\n", (void*) dlerror_str_addr);
            if (dlerror_str_addr == 0) return false;
            auto strlen_addr = find_func_addr(local_map, map, "libc.so", "strlen");
            if (strlen_addr == nullptr) {
                ZLOGE("find strlen\n");
                return false;
            }
            args.clear();
            args.push_back(dlerror_str_addr);
            auto dlerror_len = remote_call(pid, regs, (uintptr_t) strlen_addr, (uintptr_t) libc_return_addr, args);
            ZLOGD("dlerror len %ld\n", dlerror_len);
            if (dlerror_len <= 0) return false;
            std::string err;
            err.resize(dlerror_len + 1, 0);
            read_proc(pid, (uintptr_t*) dlerror_str_addr, err.data(), dlerror_len);
            ZLOGE("dlerror info %s\n", err.c_str());
            return false;
        }

        // call dlsym(handle, "entry")
        auto dlsym_addr = find_func_addr(local_map, map, "libdl.so", "dlsym");
        if (dlsym_addr == nullptr) return false;
        args.clear();
        str = push_string(pid, regs, "zygisk_inject_entry");
        args.push_back(remote_handle);
        args.push_back((long) str);
        auto injector_entry = remote_call(pid, regs, (uintptr_t) dlsym_addr, (uintptr_t) libc_return_addr, args);
        ZLOGD("injector entry %p\n", (void*) injector_entry);
        if (injector_entry == 0) {
            ZLOGE("injector entry is null\n");
            return false;
        }

        // call injector entry(handle)
        args.clear();
        args.push_back(remote_handle);
        remote_call(pid, regs, injector_entry, (uintptr_t) libc_return_addr, args);

        // reset pc to entry
        backup.REG_IP = (long) entry_addr;
        ZLOGD("invoke entry\n");
        // restore registers
        if (!set_regs(pid, backup)) return false;

        return true;
    } else {
        ZLOGE("stopped by other reason: %s\n", parse_status(status).c_str());
    }
    return false;
}

#define STOPPED_WITH(sig, event) (WIFSTOPPED(status) && WSTOPSIG(status) == (sig) && (status >> 16) == (event))

bool trace_zygote(int pid, const char *libpath) {
    ZLOGI("start tracing %d\n", pid);
#define WAIT_OR_DIE wait_for_trace(pid, &status, __WALL);
#define CONT_OR_DIE \
    if (ptrace(PTRACE_CONT, pid, 0, 0) == -1) { \
        PLOGE("cont"); \
        return false; \
    }
    int status;
    ZLOGI("tracing %d (tracer %d)\n", pid, getpid());
    if (ptrace(PTRACE_SEIZE, pid, 0, PTRACE_O_EXITKILL) == -1) {
        PLOGE("seize");
        return false;
    }
    WAIT_OR_DIE
    if (STOPPED_WITH(SIGSTOP, PTRACE_EVENT_STOP)) {
        bool is_injected = inject_on_main(pid, libpath);

        if (!is_injected) {
            ZLOGE("failed to inject\n");
            return false;
        }
        ZLOGD("inject done, continue process\n");
        if (kill(pid, SIGCONT)) {
            PLOGE("kill");
            return false;
        }
        CONT_OR_DIE
        WAIT_OR_DIE
        if (STOPPED_WITH(SIGTRAP, PTRACE_EVENT_STOP)) {
            CONT_OR_DIE
            WAIT_OR_DIE
            if (STOPPED_WITH(SIGCONT, 0)) {
                ZLOGD("received SIGCONT\n");
                ptrace(PTRACE_DETACH, pid, 0, SIGCONT);
            }
        } else {
            ZLOGE("unknown state %s, not SIGTRAP + EVENT_STOP\n", parse_status(status).c_str());
            ptrace(PTRACE_DETACH, pid, 0, 0);
            return false;
        }
    } else {
        ZLOGE("unknown state %s, not SIGSTOP + EVENT_STOP\n", parse_status(status).c_str());
        ptrace(PTRACE_DETACH, pid, 0, 0);
        return false;
    }
    return true;
}
