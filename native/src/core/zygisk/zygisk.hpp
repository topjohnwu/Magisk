#pragma once

#include <jni.h>
#include <core.hpp>

#define ZYGISKLDR       "libzygisk.so"
#define NBPROP          "ro.dalvik.vm.native.bridge"

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

// Extreme verbose logging
#define ZLOGV(...) ZLOGD(__VA_ARGS__)
//#define ZLOGV(...) (void*)0

void hook_entry();
void hookJniNativeMethods(JNIEnv *env, const char *clz, JNINativeMethod *methods, int numMethods);

inline int zygisk_request(int req) {
    int fd = connect_daemon(+RequestCode::ZYGISK);
    if (fd < 0) return fd;
    write_int(fd, req);
    return fd;
}

// The reference of the following structs
// https://cs.android.com/android/platform/superproject/main/+/main:art/libnativebridge/include/nativebridge/native_bridge.h

struct NativeBridgeRuntimeCallbacks {
    const char* (*getMethodShorty)(JNIEnv* env, jmethodID mid);
    uint32_t (*getNativeMethodCount)(JNIEnv* env, jclass clazz);
    uint32_t (*getNativeMethods)(JNIEnv* env, jclass clazz, JNINativeMethod* methods,
                                 uint32_t method_count);
};

struct NativeBridgeCallbacks {
    uint32_t version;
    void *padding[5];
    bool (*isCompatibleWith)(uint32_t);
};
