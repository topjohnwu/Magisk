#pragma once

#include <sys/mman.h>
#include <stdint.h>
#include <jni.h>
#include <vector>
#include <daemon.hpp>

namespace ZygiskRequest {
enum : int {
    GET_INFO,
    CONNECT_COMPANION,
    GET_MODDIR,
    END
};
}

#if defined(__LP64__)
#define ZLOGD(...) LOGD("zygisk64: " __VA_ARGS__)
#define ZLOGE(...) LOGE("zygisk64: " __VA_ARGS__)
#define ZLOGI(...) LOGI("zygisk64: " __VA_ARGS__)
#define ZLOGW(...) LOGW("zygisk64: " __VA_ARGS__)
#else
#define ZLOGD(...) LOGD("zygisk32: " __VA_ARGS__)
#define ZLOGE(...) LOGE("zygisk32: " __VA_ARGS__)
#define ZLOGI(...) LOGI("zygisk32: " __VA_ARGS__)
#define ZLOGW(...) LOGW("zygisk32: " __VA_ARGS__)
#endif

extern void *self_handle;

void hook_functions();
int remote_get_info(int uid, const char *process, uint32_t *flags, std::vector<int> &fds);

inline int zygisk_request(int req) {
    int fd = connect_daemon(MainRequest::ZYGISK);
    if (fd < 0) return fd;
    write_int(fd, req);
    return fd;
}
