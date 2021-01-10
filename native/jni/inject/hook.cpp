#include <jni.h>

#include <xhook.h>
#include <utils.hpp>
#include <flags.hpp>

#include "inject.hpp"

using namespace std;

static JavaVM *g_jvm;

// For some reason static vector won't work, use a pointer instead
static vector<tuple<const char *, const char *, void **>> *hook_list;

namespace {

struct HookContext {
    int pid;
};

}

// JNI method declarations

namespace JNI {
    namespace Zygote {
        const JNINativeMethod *nativeForkAndSpecialize_orig = nullptr;
        const JNINativeMethod *nativeSpecializeAppProcess_orig = nullptr;
        const JNINativeMethod *nativeForkSystemServer_orig = nullptr;

        extern const JNINativeMethod nativeForkAndSpecialize_methods[];
        extern const int nativeForkAndSpecialize_methods_num;

        extern const JNINativeMethod nativeSpecializeAppProcess_methods[];
        extern const int nativeSpecializeAppProcess_methods_num;

        extern const JNINativeMethod nativeForkSystemServer_methods[];
        extern const int nativeForkSystemServer_methods_num;
    }
    namespace SystemProperties {
        const JNINativeMethod *native_set_orig = nullptr;

        extern const JNINativeMethod native_set_methods[];
        constexpr int native_set_methods_num = 1;
    }
}

#define DEF_HOOK_FUNC(ret, func, ...) \
    static ret (*old_##func)(__VA_ARGS__); \
    static ret new_##func(__VA_ARGS__)

#define HOOK_JNI(clazz, method) \
if (newMethods[i].name == #method##sv) { \
    auto orig = new JNINativeMethod(); \
    memcpy(orig, &newMethods[i], sizeof(JNINativeMethod)); \
    JNI::clazz::method##_orig = orig; \
    for (int j = 0; j < JNI::clazz::method##_methods_num; ++j) { \
        if (strcmp(newMethods[i].signature, JNI::clazz::method##_methods[j].signature) == 0) { \
            newMethods[i] = JNI::clazz::method##_methods[j]; \
            LOGI("hook: replaced " #clazz "#" #method "\n"); \
            ++hooked; \
            break; \
        } \
    } \
    continue; \
}

#define clone_methods() \
    newMethods = make_unique<JNINativeMethod[]>(numMethods); \
    memcpy(newMethods.get(), methods, sizeof(JNINativeMethod) * numMethods)

DEF_HOOK_FUNC(int, jniRegisterNativeMethods,
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {
    LOGD("hook: jniRegisterNativeMethods %s", className);

    unique_ptr<JNINativeMethod[]> newMethods;
    int hooked = 0;

    if (g_jvm == nullptr) {
        // Save for later unhooking
        env->GetJavaVM(&g_jvm);
    }

    if (className == "com/android/internal/os/Zygote"sv) {
        clone_methods();
        for (int i = 0; i < numMethods && hooked < 3; ++i) {
            HOOK_JNI(Zygote, nativeForkAndSpecialize);
            HOOK_JNI(Zygote, nativeSpecializeAppProcess);
            HOOK_JNI(Zygote, nativeForkSystemServer);
        }
    } else if (className == "android/os/SystemProperties"sv) {
        clone_methods();
        for (int i = 0; i < numMethods && hooked < 1; ++i) {
            HOOK_JNI(SystemProperties, native_set);
        }
    }

    return old_jniRegisterNativeMethods(env, className, newMethods.get() ?: methods, numMethods);
}

static void nativeForkAndSpecialize_pre(HookContext *ctx,
        JNIEnv *env, jclass clazz, jint &uid, jint &gid, jintArray &gids, jint &runtime_flags,
        jobjectArray &rlimits, jint &mount_external, jstring &se_info, jstring &nice_name,
        jintArray &fds_to_close, jintArray &fds_to_ignore, jboolean &is_child_zygote,
        jstring &instruction_set, jstring &app_data_dir, jboolean &is_top_app, jobjectArray &pkg_data_info_list,
        jobjectArray &whitelisted_data_info_list, jboolean &mount_data_dirs, jboolean &mount_storage_dirs) {
    LOGD("hook: %s\n", __FUNCTION__);
}

static void nativeForkAndSpecialize_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    LOGD("hook: %s\n", __FUNCTION__);
    // Demonstrate self unload in child process
    if (ctx->pid == 0)
        self_unload();
}

// -----------------------------------------------------------------

static void nativeSpecializeAppProcess_pre(HookContext *ctx,
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jboolean is_child_zygote, jstring instruction_set, jstring app_data_dir,
        jboolean &is_top_app, jobjectArray &pkg_data_info_list, jobjectArray &whitelisted_data_info_list,
        jboolean &mount_data_dirs, jboolean &mount_storage_dirs) {
    LOGD("hook: %s\n", __FUNCTION__);
}

static void nativeSpecializeAppProcess_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    LOGD("hook: %s\n", __FUNCTION__);
}

// -----------------------------------------------------------------

static void nativeForkSystemServer_pre(HookContext *ctx,
        JNIEnv *env, jclass clazz, uid_t &uid, gid_t &gid, jintArray &gids, jint &runtime_flags,
        jobjectArray &rlimits, jlong &permitted_capabilities, jlong &effective_capabilities) {
    LOGD("hook: %s\n", __FUNCTION__);
}

static void nativeForkSystemServer_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    LOGD("hook: %s\n", __FUNCTION__);
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

#define push_method(clazz, method) \
if (JNI::clazz::method##_orig) methods.emplace_back(*JNI::clazz::method##_orig)

bool unhook_functions() {
    JNIEnv* env;
    if (g_jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
        return false;

    // Unhook JNI methods
    vector<JNINativeMethod> methods;

    push_method(Zygote, nativeForkAndSpecialize);
    push_method(Zygote, nativeSpecializeAppProcess);
    push_method(Zygote, nativeForkSystemServer);

    if (!methods.empty() && old_jniRegisterNativeMethods(env,
            "com/android/internal/os/Zygote",
            methods.data(), methods.size()) != 0) {
        LOGE("hook: Failed to register JNI hook for Zygote\n");
        return false;
    }

    methods.clear();
    push_method(SystemProperties, native_set);

    if (!methods.empty() && old_jniRegisterNativeMethods(env,
            "android/os/SystemProperties",
            methods.data(), methods.size()) != 0) {
        LOGE("hook: Failed to register JNI hook for SystemProperties\n");
        return false;
    }

    // Unhook xhook
    for (auto &[path, sym, old_func] : *hook_list) {
        if (xhook_register(path, sym, *old_func, nullptr) != 0) {
            LOGE("hook: Failed to register hook \"%s\"\n", sym);
            return false;
        }
    }
    delete hook_list;
    return hook_refresh();
}

#include "jni_hooks.hpp"
