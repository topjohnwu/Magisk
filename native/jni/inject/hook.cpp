#include <xhook.h>
#include <utils.hpp>
#include <flags.hpp>
#include <daemon.hpp>

#include "inject.hpp"
#include "memory.hpp"

using namespace std;
using jni_hook::hash_map;
using jni_hook::tree_map;
using xstring = jni_hook::string;

struct SpecializeAppProcessArgs {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jint &mount_external;
    jstring &se_info;
    jstring &nice_name;
    jstring &instruction_set;
    jstring &app_data_dir;

    /* Optional */
    jboolean *is_child_zygote = nullptr;
    jboolean *is_top_app = nullptr;
    jobjectArray *pkg_data_info_list = nullptr;
    jobjectArray *whitelisted_data_info_list = nullptr;
    jboolean *mount_data_dirs = nullptr;
    jboolean *mount_storage_dirs = nullptr;

    SpecializeAppProcessArgs(
            jint &uid, jint &gid, jintArray &gids, jint &runtime_flags,
            jint &mount_external, jstring &se_info, jstring &nice_name,
            jstring &instruction_set, jstring &app_data_dir) :
            uid(uid), gid(gid), gids(gids), runtime_flags(runtime_flags),
            mount_external(mount_external), se_info(se_info), nice_name(nice_name),
            instruction_set(instruction_set), app_data_dir(app_data_dir) {}
};

struct ForkSystemServerArgs {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jlong &permitted_capabilities;
    jlong &effective_capabilities;

    ForkSystemServerArgs(
            jint &uid, jint &gid, jintArray &gids, jint &runtime_flags,
            jlong &permitted_capabilities, jlong &effective_capabilities) :
            uid(uid), gid(gid), gids(gids), runtime_flags(runtime_flags),
            permitted_capabilities(permitted_capabilities),
            effective_capabilities(effective_capabilities) {}
};

struct HookContext {
    int pid;
    bool do_hide;
    union {
        SpecializeAppProcessArgs *args;
        ForkSystemServerArgs *server_args;
        void *raw_args;
    };
};

static vector<tuple<const char *, const char *, void **>> *xhook_list;
static vector<JNINativeMethod> *jni_hook_list;
static hash_map<xstring, tree_map<xstring, tree_map<xstring, void *>>> *jni_method_map;

static JavaVM *g_jvm;
static HookContext *current_ctx;

#define DCL_HOOK_FUNC(ret, func, ...) \
    static ret (*old_##func)(__VA_ARGS__); \
    static ret new_##func(__VA_ARGS__)

#define DCL_JNI_FUNC(name) \
    static void *name##_orig; \
    extern const JNINativeMethod name##_methods[];       \
    extern const int name##_methods_num;

namespace {
// JNI method declarations
DCL_JNI_FUNC(nativeForkAndSpecialize)
DCL_JNI_FUNC(nativeSpecializeAppProcess)
DCL_JNI_FUNC(nativeForkSystemServer)
}

#define HOOK_JNI(method) \
if (methods[i].name == #method##sv) { \
    jni_hook_list->push_back(methods[i]);              \
    method##_orig = methods[i].fnPtr; \
    for (int j = 0; j < method##_methods_num; ++j) {   \
        if (strcmp(methods[i].signature, method##_methods[j].signature) == 0) { \
            newMethods[i] = method##_methods[j];       \
            LOGI("hook: replaced #" #method "\n");     \
            ++hooked;    \
            break;       \
        }                \
    }                    \
    continue;            \
}

DCL_HOOK_FUNC(int, jniRegisterNativeMethods,
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {
    LOGD("hook: jniRegisterNativeMethods %s", className);

    if (g_jvm == nullptr) {
        // Save for later unhooking
        env->GetJavaVM(&g_jvm);
    }

    unique_ptr<JNINativeMethod[]> newMethods;
    int hooked = numeric_limits<int>::max();
    if (className == "com/android/internal/os/Zygote"sv) {
        hooked = 0;
        newMethods = make_unique<JNINativeMethod[]>(numMethods);
        memcpy(newMethods.get(), methods, sizeof(JNINativeMethod) * numMethods);
    }

    auto &class_map = (*jni_method_map)[className];
    for (int i = 0; i < numMethods; ++i) {
        class_map[methods[i].name][methods[i].signature] = methods[i].fnPtr;
        if (hooked < 3) {
            HOOK_JNI(nativeForkAndSpecialize);
            HOOK_JNI(nativeSpecializeAppProcess);
            HOOK_JNI(nativeForkSystemServer);
        }
    }

    return old_jniRegisterNativeMethods(env, className, newMethods.get() ?: methods, numMethods);
}

DCL_HOOK_FUNC(int, fork) {
    return current_ctx ? current_ctx->pid : old_fork();
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

// -----------------------------------------------------------------

static void nativeSpecializeAppProcess_pre(HookContext *ctx, JNIEnv *env, jclass clazz) {
    current_ctx = ctx;
    const char *process = env->GetStringUTFChars(ctx->args->nice_name, nullptr);
    LOGD("hook: %s %s\n", __FUNCTION__, process);

    if (ctx->args->mount_external != 0  /* TODO: Handle MOUNT_EXTERNAL_NONE cases */
        && remote_check_hide(ctx->args->uid, process)) {
        ctx->do_hide = true;
        LOGI("hook: [%s] should be hidden\n", process);
    }

    env->ReleaseStringUTFChars(ctx->args->nice_name, process);
}

static void nativeSpecializeAppProcess_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    current_ctx = nullptr;
    LOGD("hook: %s\n", __FUNCTION__);

    if (ctx->do_hide)
        self_unload();
}

// -----------------------------------------------------------------

// Do our own fork before loading any 3rd party code
// First block SIGCHLD, unblock after original fork is done
#define PRE_FORK() \
    current_ctx = ctx; \
    sigmask(SIG_BLOCK, SIGCHLD); \
    ctx->pid = old_fork();       \
    if (ctx->pid != 0)           \
        return;

// Unblock SIGCHLD in case the original method didn't
#define POST_FORK() \
    current_ctx = nullptr; \
    sigmask(SIG_UNBLOCK, SIGCHLD); \
    if (ctx->pid != 0)\
        return;

static void nativeForkAndSpecialize_pre(HookContext *ctx, JNIEnv *env, jclass clazz) {
    PRE_FORK();
    nativeSpecializeAppProcess_pre(ctx, env, clazz);
}

static void nativeForkAndSpecialize_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    POST_FORK();
    nativeSpecializeAppProcess_post(ctx, env, clazz);
}

// -----------------------------------------------------------------

static void nativeForkSystemServer_pre(HookContext *ctx, JNIEnv *env, jclass clazz) {
    PRE_FORK();
    LOGD("hook: %s\n", __FUNCTION__);
}

static void nativeForkSystemServer_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    POST_FORK();
    LOGD("hook: %s\n", __FUNCTION__);
}

#undef PRE_FORK
#undef POST_FORK

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
    jni_hook_list = new remove_pointer_t<decltype(jni_hook_list)>();
    jni_method_map = new remove_pointer_t<decltype(jni_method_map)>();

    XHOOK_REGISTER(".*\\libandroid_runtime.so$", jniRegisterNativeMethods);
    XHOOK_REGISTER(".*\\libandroid_runtime.so$", fork);
    XHOOK_REGISTER(".*\\libandroid_runtime.so$", selinux_android_setcontext);
    hook_refresh();
}

bool unhook_functions() {
    JNIEnv* env;
    if (g_jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
        return false;

    // Do NOT call any destructors
    operator delete(jni_method_map);
    // Directly unmap the whole memory block
    jni_hook::memory_block::release();

    // Unhook JNI methods
    if (!jni_hook_list->empty() && old_jniRegisterNativeMethods(env,
            "com/android/internal/os/Zygote",
            jni_hook_list->data(), jni_hook_list->size()) != 0) {
        LOGE("hook: Failed to register JNI hook\n");
        return false;
    }
    delete jni_hook_list;

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

// JNI method definitions, include all method signatures of past Android versions
#include "jni_hooks.hpp"
