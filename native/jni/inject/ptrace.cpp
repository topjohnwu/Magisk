/*
 * Original code: https://github.com/Chainfire/injectvm-binderjack/blob/master/app/src/main/jni/libinject/inject.cpp
 * The code is heavily modified and sublicensed to GPLv3 for incorporating into Magisk.
 *
 * Copyright (c) 2015, Simone 'evilsocket' Margaritelli
 * Copyright (c) 2015-2019, Jorrit 'Chainfire' Jongma
 * Copyright (c) 2021, John 'topjohnwu' Wu
 *
 * See original LICENSE file from the original project for additional details:
 * https://github.com/Chainfire/injectvm-binderjack/blob/master/LICENSE
 */

/*
 * NOTE:
 * The code in this file was originally planned to be used for some features,
 * but it turned out to be unsuitable for the task. However, this shall remain
 * in our arsenal in case it may be used in the future.
 */

#include <elf.h>
#include <sys/mount.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <utils.hpp>

#include "inject.hpp"
#include "ptrace.hpp"

using namespace std;

#if defined(__arm__)
#define CPSR_T_MASK (1u << 5)
#define PARAMS_IN_REGS 4
#elif defined(__aarch64__)
#define CPSR_T_MASK (1u << 5)
#define PARAMS_IN_REGS 8
#define pt_regs user_pt_regs
#define uregs regs
#define ARM_pc pc
#define ARM_sp sp
#define ARM_cpsr pstate
#define ARM_lr regs[30]
#define ARM_r0 regs[0]
#endif

bool _remote_read(int pid, uintptr_t addr, void *buf, size_t len) {
    for (size_t i = 0; i < len; i += sizeof(long)) {
        long data = xptrace(PTRACE_PEEKTEXT, pid, reinterpret_cast<void*>(addr + i));
        if (data < 0)
            return false;
        memcpy(static_cast<uint8_t *>(buf) + i, &data, std::min(len - i, sizeof(data)));
    }
    return true;
}

bool _remote_write(int pid, uintptr_t addr, const void *buf, size_t len) {
    for (size_t i = 0; i < len; i += sizeof(long)) {
        long data = 0;
        memcpy(&data, static_cast<const uint8_t *>(buf) + i, std::min(len - i, sizeof(data)));
        if (xptrace(PTRACE_POKETEXT, pid, reinterpret_cast<void*>(addr + i), data) < 0)
            return false;
    }
    return true;
}

// Get remote registers
#define remote_getregs(regs) _remote_getregs(pid, regs)
static void _remote_getregs(int pid, pt_regs *regs) {
#if defined(__LP64__)
    uintptr_t regset = NT_PRSTATUS;
    iovec iov{};
    iov.iov_base = regs;
    iov.iov_len = sizeof(*regs);
    xptrace(PTRACE_GETREGSET, pid, reinterpret_cast<void*>(regset), &iov);
#else
    xptrace(PTRACE_GETREGS, pid, nullptr, regs);
#endif
}

// Set remote registers
#define remote_setregs(regs) _remote_setregs(pid, regs)
static void _remote_setregs(int pid, pt_regs *regs) {
#if defined(__LP64__)
    uintptr_t regset = NT_PRSTATUS;
    iovec iov{};
    iov.iov_base = regs;
    iov.iov_len = sizeof(*regs);
    xptrace(PTRACE_SETREGSET, pid, reinterpret_cast<void*>(regset), &iov);
#else
    xptrace(PTRACE_SETREGS, pid, nullptr, regs);
#endif
}

uintptr_t remote_call_abi(int pid, uintptr_t func_addr, int nargs, va_list va) {
    pt_regs regs, regs_bak;

    // Get registers and save a backup
    remote_getregs(&regs);
    memcpy(&regs_bak, &regs, sizeof(regs));

    // ABI dependent: Setup stack and registers to perform the call

#if defined(__arm__) || defined(__aarch64__)
    // Fill R0-Rx with the first 4 (32-bit) or 8 (64-bit) parameters
    for (int i = 0; (i < nargs) && (i < PARAMS_IN_REGS); ++i) {
        regs.uregs[i] = va_arg(va, uintptr_t);
    }

    // Push remaining parameters onto stack
    if (nargs > PARAMS_IN_REGS) {
        regs.ARM_sp -= sizeof(uintptr_t) * (nargs - PARAMS_IN_REGS);
        uintptr_t stack = regs.ARM_sp;
        for (int i = PARAMS_IN_REGS; i < nargs; ++i) {
            uintptr_t arg = va_arg(va, uintptr_t);
            remote_write(stack, &arg, sizeof(uintptr_t));
            stack += sizeof(uintptr_t);
        }
    }

    // Set return address
    regs.ARM_lr = 0;

    // Set function address to call
    regs.ARM_pc = func_addr;

    // Setup the current processor status register
    if (regs.ARM_pc & 1u) {
        // thumb
        regs.ARM_pc &= (~1u);
        regs.ARM_cpsr |= CPSR_T_MASK;
    } else {
        // arm
        regs.ARM_cpsr &= ~CPSR_T_MASK;
    }
#elif defined(__i386__)
    // Push all params onto stack
    regs.esp -= sizeof(uintptr_t) * nargs;
    uintptr_t stack = regs.esp;
    for (int i = 0; i < nargs; ++i) {
        uintptr_t arg = va_arg(va, uintptr_t);
        remote_write(stack, &arg, sizeof(uintptr_t));
        stack += sizeof(uintptr_t);
    }

    // Push return address onto stack
    uintptr_t ret_addr = 0;
    regs.esp -= sizeof(uintptr_t);
    remote_write(regs.esp, &ret_addr, sizeof(uintptr_t));

    // Set function address to call
    regs.eip = func_addr;
#elif defined(__x86_64__)
    // Align, rsp - 8 must be a multiple of 16 at function entry point
    uintptr_t space = sizeof(uintptr_t);
    if (nargs > 6)
        space += sizeof(uintptr_t) * (nargs - 6);
    while (((regs.rsp - space - 8) & 0xF) != 0)
        regs.rsp--;

    // Fill [RDI, RSI, RDX, RCX, R8, R9] with the first 6 parameters
    for (int i = 0; (i < nargs) && (i < 6); ++i) {
        uintptr_t arg = va_arg(va, uintptr_t);
        switch (i) {
            case 0: regs.rdi = arg; break;
            case 1: regs.rsi = arg; break;
            case 2: regs.rdx = arg; break;
            case 3: regs.rcx = arg; break;
            case 4: regs.r8 = arg; break;
            case 5: regs.r9 = arg; break;
        }
    }

    // Push remaining parameters onto stack
    if (nargs > 6) {
        regs.rsp -= sizeof(uintptr_t) * (nargs - 6);
        uintptr_t stack = regs.rsp;
        for(int i = 6; i < nargs; ++i) {
            uintptr_t arg = va_arg(va, uintptr_t);
            remote_write(stack, &arg, sizeof(uintptr_t));
            stack += sizeof(uintptr_t);
        }
    }

    // Push return address onto stack
    uintptr_t ret_addr = 0;
    regs.rsp -= sizeof(uintptr_t);
    remote_write(regs.rsp, &ret_addr, sizeof(uintptr_t));

    // Set function address to call
    regs.rip = func_addr;

    // may be needed
    regs.rax = 0;
    regs.orig_rax = 0;
#else
#error Unsupported ABI
#endif

    // Resume process to do the call
    remote_setregs(&regs);
    xptrace(PTRACE_CONT, pid);

    // Catch SIGSEGV caused by the 0 return address
    int status;
    while (waitpid(pid, &status, __WALL | __WNOTHREAD) == pid) {
        if (WIFSTOPPED(status) && (WSTOPSIG(status) == SIGSEGV))
            break;
        xptrace(PTRACE_CONT, pid);
    }

    // Get registers again for return value
    remote_getregs(&regs);

    // Restore registers
    remote_setregs(&regs_bak);

#if defined(__arm__) || defined(__aarch64__)
    return regs.ARM_r0;
#elif defined(__i386__)
    return regs.eax;
#elif defined(__x86_64__)
    return regs.rax;
#endif
}

uintptr_t remote_call_vararg(int pid, uintptr_t addr, int nargs, ...) {
    char lib_name[4096];
    auto local = get_function_lib(addr, lib_name);
    if (local == 0)
        return 0;
    auto remote = get_remote_lib(pid, lib_name);
    if (remote == 0)
        return 0;
    addr = addr - local + remote;
    va_list va;
    va_start(va, nargs);
    auto result = remote_call_abi(pid, addr, nargs, va);
    va_end(va);
    return result;
}
