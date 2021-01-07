#pragma once

#include <stdint.h>

// Write bytes to the remote process at addr
bool _remote_write(int pid, uintptr_t addr, const void *buf, size_t len);
#define remote_write(...) _remote_write(pid, __VA_ARGS__)

// Read bytes from the remote process at addr
bool _remote_read(int pid, uintptr_t addr, void *buf, size_t len);
#define remote_read(...) _remote_read(pid, __VA_ARGS__)

// Call a remote function
// Arguments are expected to be only integer-like or pointer types
// as other more complex C ABIs are not implemented.
uintptr_t remote_call_abi(int pid, uintptr_t func_addr, int nargs, va_list va);

// Find remote offset and invoke function
uintptr_t remote_call_vararg(int pid, uintptr_t addr, int nargs, ...);

// C++ wrapper for auto argument counting and casting function pointers
template<class FuncPtr, class ...Args>
static uintptr_t _remote_call(int pid, FuncPtr sym, Args && ...args) {
    auto addr = reinterpret_cast<uintptr_t>(sym);
    return remote_call_vararg(pid, addr, sizeof...(args), std::forward<Args>(args)...);
}
#define remote_call(...) _remote_call(pid, __VA_ARGS__)
