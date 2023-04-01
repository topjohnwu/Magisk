#pragma once

#include <stdint.h>
#include <jni.h>
#include <vector>
#include <daemon.hpp>

#define MAGISKTMP_ENV  "MAGISKTMP"

#define HIJACK_BIN64   "/system/bin/appwidget"
#define HIJACK_BIN32   "/system/bin/bu"

namespace ZygiskRequest {
enum : int {
    SETUP,
    GET_INFO,
    GET_LOG_PIPE,
    CONNECT_COMPANION,
    GET_MODDIR,
    PASSTHROUGH,
    END
};
}

#if defined(__LP64__)
#define ZLOGD(...) LOGD("zygisk64: " __VA_ARGS__)
#define ZLOGE(...) LOGE("zygisk64: " __VA_ARGS__)
#define ZLOGI(...) LOGI("zygisk64: " __VA_ARGS__)
#define ZLOGW(...) LOGW("zygisk64: " __VA_ARGS__)
#define HIJACK_BIN HIJACK_BIN64
#else
#define ZLOGD(...) LOGD("zygisk32: " __VA_ARGS__)
#define ZLOGE(...) LOGE("zygisk32: " __VA_ARGS__)
#define ZLOGI(...) LOGI("zygisk32: " __VA_ARGS__)
#define ZLOGW(...) LOGW("zygisk32: " __VA_ARGS__)
#define HIJACK_BIN HIJACK_BIN32
#endif

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

inline int zygisk_request(int req) {
    int fd = connect_daemon(MainRequest::ZYGISK);
    if (fd < 0) return fd;
    write_int(fd, req);
    return fd;
}
