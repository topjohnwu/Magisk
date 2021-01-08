#include <jni.h>

#include <xhook.h>
#include <utils.hpp>
#include <flags.hpp>

#include "inject.hpp"

using namespace std;

// Static vector won't work, use a pointer instead
static vector<tuple<const char *, const char *, void **>> *hook_list;

#define DEF_HOOK_FUNC(ret, func, ...) \
    static ret (*old_##func)(__VA_ARGS__); \
    static ret new_##func(__VA_ARGS__)

DEF_HOOK_FUNC(int, jniRegisterNativeMethods,
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {
    LOGD("hook: jniRegisterNativeMethods %s", className);

    // TODO: actually do things like replacing JNI native methods

    return old_jniRegisterNativeMethods(env, className, methods, numMethods);
}

static bool hook_refresh() {
    if (xhook_refresh(0) == 0) {
        xhook_clear();
        LOGI("hook: xhook success\n");
        return true;
    } else {
        LOGE("hook: xhook failed\n");
        return false;
    }
}

static int hook_register(const char *path, const char *symbol, void *new_func, void **old_func) {
    int ret = xhook_register(path, symbol, new_func, old_func);
    if (ret != 0) {
        LOGE("hook: Failed to register hook \"%s\"\n", symbol);
        return ret;
    }
    hook_list->emplace_back(path, symbol, old_func);
    return 0;
}

#define XHOOK_REGISTER(PATH_REGEX, NAME) \
    hook_register(PATH_REGEX, #NAME, (void*) new_##NAME, (void **) &old_##NAME)

void hook_functions() {
#ifdef MAGISK_DEBUG
    xhook_enable_debug(1);
    xhook_enable_sigsegv_protection(0);
#endif
    hook_list = new remove_pointer_t<decltype(hook_list)>();
    XHOOK_REGISTER(".*\\libandroid_runtime.so$", jniRegisterNativeMethods);
    hook_refresh();
}

bool unhook_functions() {
    for (auto &[path, sym, old_func] : *hook_list) {
        if (xhook_register(path, sym, *old_func, nullptr) != 0) {
            LOGE("hook: Failed to register hook \"%s\"\n", sym); \
            return false;
        }
    }
    delete hook_list;
    return hook_refresh();
}
