#include <jni.h>

#include <xhook.h>
#include <utils.hpp>
#include <flags.hpp>
#include <daemon.hpp>

#include "inject.hpp"

using namespace std;

#define DCL_HOOK_FUNC(ret, func, ...) \
    static ret (*old_##func)(__VA_ARGS__); \
    static ret new_##func(__VA_ARGS__)

#define DCL_JNI_FUNC(name) \
    static const JNINativeMethod *name##_orig = nullptr; \
    extern const JNINativeMethod name##_methods[]; \
    extern const int name##_methods_num;

namespace {

struct HookContext {
    int pid;
    bool do_hide;
};

// JNI method declarations
DCL_JNI_FUNC(nativeForkAndSpecialize)
DCL_JNI_FUNC(nativeSpecializeAppProcess)
DCL_JNI_FUNC(nativeForkSystemServer)

}

// For some reason static vectors won't work, use pointers instead
static vector<tuple<const char *, const char *, void **>> *xhook_list;
static vector<JNINativeMethod> *jni_list;

static JavaVM *g_jvm;
static int prev_fork_pid = -1;
static HookContext *current_ctx;

#define HOOK_JNI(method) \
if (newMethods[i].name == #method##sv) { \
    auto orig = new JNINativeMethod(); \
    memcpy(orig, &newMethods[i], sizeof(JNINativeMethod)); \
    method##_orig = orig; \
    jni_list->push_back(newMethods[i]); \
    for (int j = 0; j < method##_methods_num; ++j) { \
        if (strcmp(newMethods[i].signature, method##_methods[j].signature) == 0) { \
            newMethods[i] = method##_methods[j]; \
            LOGI("hook: replaced #" #method "\n"); \
            ++hooked; \
            break; \
        } \
    } \
    continue; \
}

DCL_HOOK_FUNC(int, jniRegisterNativeMethods,
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {
    LOGD("hook: jniRegisterNativeMethods %s", className);

    unique_ptr<JNINativeMethod[]> newMethods;
    int hooked = 0;

    if (g_jvm == nullptr) {
        // Save for later unhooking
        env->GetJavaVM(&g_jvm);
    }

    if (className == "com/android/internal/os/Zygote"sv) {
        newMethods = make_unique<JNINativeMethod[]>(numMethods);
        memcpy(newMethods.get(), methods, sizeof(JNINativeMethod) * numMethods);
        for (int i = 0; i < numMethods && hooked < 3; ++i) {
            HOOK_JNI(nativeForkAndSpecialize);
            HOOK_JNI(nativeSpecializeAppProcess);
            HOOK_JNI(nativeForkSystemServer);
        }
    }

    return old_jniRegisterNativeMethods(env, className, newMethods.get() ?: methods, numMethods);
}

DCL_HOOK_FUNC(int, fork) {
    if (prev_fork_pid < 0)
        return old_fork();

    // Skip an actual fork and return the previous fork result
    int pid = prev_fork_pid;
    prev_fork_pid = -1;
    return pid;
}

DCL_HOOK_FUNC(int, selinux_android_setcontext,
        uid_t uid, int isSystemServer, const char *seinfo, const char *pkgname) {
    if (current_ctx && current_ctx->do_hide) {
        // Ask magiskd to hide ourselves before switching context
        // because magiskd socket is not accessible on Android 8.0+
        remote_request_hide();
        LOGD("hook: process successfully hidden\n");
    }
    return old_selinux_android_setcontext(uid, isSystemServer, seinfo, pkgname);
}

static int sigmask(int how, int signum) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, signum);
    return sigprocmask(how, &set, nullptr);
}

static int pre_specialize_fork() {
    // First block SIGCHLD, unblock after original fork is done
    sigmask(SIG_BLOCK, SIGCHLD);
    prev_fork_pid = old_fork();
    return prev_fork_pid;
}

// -----------------------------------------------------------------

static void nativeSpecializeAppProcess_pre(HookContext *ctx,
        JNIEnv *env, jclass clazz, jint &uid, jint &gid, jintArray &gids, jint &runtime_flags,
        jobjectArray &rlimits, jint &mount_external, jstring &se_info, jstring &nice_name,
        jboolean &is_child_zygote, jstring &instruction_set, jstring &app_data_dir,
        jboolean &is_top_app, jobjectArray &pkg_data_info_list,
        jobjectArray &whitelisted_data_info_list, jboolean &mount_data_dirs,
        jboolean &mount_storage_dirs) {

    current_ctx = ctx;

    const char *process = env->GetStringUTFChars(nice_name, nullptr);
    LOGD("hook: %s %s\n", __FUNCTION__, process);

    if (mount_external != 0  /* TODO: Handle MOUNT_EXTERNAL_NONE cases */
        && remote_check_hide(uid, process)) {
        ctx->do_hide = true;
        LOGI("hook: [%s] should be hidden\n", process);
    }

    env->ReleaseStringUTFChars(nice_name, process);
}

static void nativeSpecializeAppProcess_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    LOGD("hook: %s\n", __FUNCTION__);

    if (ctx->do_hide)
        self_unload();

    current_ctx = nullptr;
}

// -----------------------------------------------------------------

static void nativeForkAndSpecialize_pre(HookContext *ctx,
        JNIEnv *env, jclass clazz, jint &uid, jint &gid, jintArray &gids, jint &runtime_flags,
        jobjectArray &rlimits, jint &mount_external, jstring &se_info, jstring &nice_name,
        jintArray fds_to_close, jintArray fds_to_ignore,  /* These 2 arguments are unique to fork */
        jboolean &is_child_zygote, jstring &instruction_set, jstring &app_data_dir,
        jboolean &is_top_app, jobjectArray &pkg_data_info_list,
        jobjectArray &whitelisted_data_info_list, jboolean &mount_data_dirs,
        jboolean &mount_storage_dirs) {

    // Do our own fork before loading any 3rd party code
    ctx->pid = pre_specialize_fork();
    if (ctx->pid != 0)
        return;

    nativeSpecializeAppProcess_pre(
            ctx, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            nice_name, is_child_zygote, instruction_set, app_data_dir, is_top_app,
            pkg_data_info_list, whitelisted_data_info_list, mount_data_dirs, mount_storage_dirs);
}

static void nativeForkAndSpecialize_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    // Unblock SIGCHLD in case the original method didn't
    sigmask(SIG_UNBLOCK, SIGCHLD);
    if (ctx->pid != 0)
        return;

    nativeSpecializeAppProcess_post(ctx, env, clazz);
}

// -----------------------------------------------------------------

static void nativeForkSystemServer_pre(HookContext *ctx,
        JNIEnv *env, jclass clazz, uid_t &uid, gid_t &gid, jintArray &gids, jint &runtime_flags,
        jobjectArray &rlimits, jlong &permitted_capabilities, jlong &effective_capabilities) {

    // Do our own fork before loading any 3rd party code
    ctx->pid = pre_specialize_fork();
    if (ctx->pid != 0)
        return;

    current_ctx = ctx;
    LOGD("hook: %s\n", __FUNCTION__);
}

static void nativeForkSystemServer_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    // Unblock SIGCHLD in case the original method didn't
    sigmask(SIG_UNBLOCK, SIGCHLD);

    if (ctx->pid != 0)
        return;

    LOGD("hook: %s\n", __FUNCTION__);
    current_ctx = nullptr;
}

// -----------------------------------------------------------------

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
    xhook_list->emplace_back(path, symbol, old_func);
    return 0;
}

#define XHOOK_REGISTER(PATH_REGEX, NAME) \
    hook_register(PATH_REGEX, #NAME, (void*) new_##NAME, (void **) &old_##NAME)

void hook_functions() {
#ifdef MAGISK_DEBUG
    xhook_enable_debug(1);
    xhook_enable_sigsegv_protection(0);
#endif
    xhook_list = new remove_pointer_t<decltype(xhook_list)>();
    jni_list = new remove_pointer_t<decltype(jni_list)>();

    XHOOK_REGISTER(".*\\libandroid_runtime.so$", jniRegisterNativeMethods);
    XHOOK_REGISTER(".*\\libandroid_runtime.so$", fork);
    XHOOK_REGISTER(".*\\libandroid_runtime.so$", selinux_android_setcontext);
    hook_refresh();
}

bool unhook_functions() {
    JNIEnv* env;
    if (g_jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
        return false;

    // Unhook JNI methods
    if (!jni_list->empty() && old_jniRegisterNativeMethods(env,
            "com/android/internal/os/Zygote",
            jni_list->data(), jni_list->size()) != 0) {
        LOGE("hook: Failed to register JNI hook\n");
        return false;
    }
    delete jni_list;

    // Unhook xhook
    for (auto &[path, sym, old_func] : *xhook_list) {
        if (xhook_register(path, sym, *old_func, nullptr) != 0) {
            LOGE("hook: Failed to register hook \"%s\"\n", sym);
            return false;
        }
    }
    delete xhook_list;
    return hook_refresh();
}

#include "jni_hooks.hpp"
