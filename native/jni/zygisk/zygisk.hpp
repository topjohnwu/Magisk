#pragma once

#include <stdint.h>
#include <jni.h>
#include <vector>
#include <daemon.hpp>

#define INJECT_ENV_1   "MAGISK_INJ_1"
#define INJECT_ENV_2   "MAGISK_INJ_2"
#define MAGISKFD_ENV   "MAGISKFD"
#define MAGISKTMP_ENV  "MAGISKTMP"

enum class ZygiskRequest : int {
    SETUP,
    GET_INFO,
    GET_LOG_PIPE,
    CONNECT_COMPANION,
    GET_MODDIR,
    PASSTHROUGH,
    END
};

#if defined(__LP64__)
#define ZLOGD(...) LOGD("zygisk64: " __VA_ARGS__)
#define ZLOGE(...) LOGE("zygisk64: " __VA_ARGS__)
#define ZLOGI(...) LOGI("zygisk64: " __VA_ARGS__)
#else
#define ZLOGD(...) LOGD("zygisk32: " __VA_ARGS__)
#define ZLOGE(...) LOGE("zygisk32: " __VA_ARGS__)
#define ZLOGI(...) LOGI("zygisk32: " __VA_ARGS__)
#endif

// Find the memory address + size of the pages matching name + inode
std::pair<void*, size_t> find_map_range(const char *name, unsigned long inode);

// Unmap all pages matching the name
void unmap_all(const char *name);

// Remap all matching pages with anonymous pages
void remap_all(const char *name);

// Get library name + offset (from start of ELF), given function address
uintptr_t get_function_off(int pid, uintptr_t addr, char *lib);

// Get function address, given library name + offset
uintptr_t get_function_addr(int pid, const char *lib, uintptr_t off);

extern void *self_handle;

void hook_functions();
int remote_get_info(int uid, const char *process, uint32_t *flags, std::vector<int> &fds);
int remote_request_unmount();


inline int zygisk_request(ZygiskRequest req) {
    int fd = connect_daemon(DaemonRequest::ZYGISK_REQUEST, false);
    write_int(fd, static_cast<int>(req));
    return fd;
}
