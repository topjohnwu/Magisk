#include <dlfcn.h>
#include <xhook.h>
#include <bitset>

#include <utils.hpp>
#include <flags.h>
#include <daemon.hpp>

#include "inject.hpp"
#include "memory.hpp"
#include "api.hpp"

using namespace std;
using jni_hook::hash_map;
using jni_hook::tree_map;
using xstring = jni_hook::string;

// Extreme verbose logging
#define VLOG(...) LOGD(__VA_ARGS__)
//#define VLOG(...)

namespace {

enum {
    DENY_FLAG,
    FORK_AND_SPECIALIZE,
    APP_SPECIALIZE,
    SERVER_SPECIALIZE,
    FLAG_MAX
};

#define DCL_PRE_POST(name) \
void name##_pre();         \
void name##_post();

struct HookContext {
    JNIEnv *env;
    union {
        SpecializeAppProcessArgs *args;
        ForkSystemServerArgs *server_args;
        void *raw_args;
    };
    const char *process;
    int pid;
    bitset<FLAG_MAX> flags;
    AppInfo info;

    HookContext() : pid(-1), info{} {}

    static void close_fds();

    DCL_PRE_POST(fork)
    DCL_PRE_POST(run_modules)

    DCL_PRE_POST(nativeForkAndSpecialize)
    DCL_PRE_POST(nativeSpecializeAppProcess)
    DCL_PRE_POST(nativeForkSystemServer)
};

#undef DCL_PRE_POST

struct StringCmp {
    using is_transparent = void;
    bool operator()(string_view a, string_view b) const { return a < b; }
};

// Global variables
vector<tuple<const char *, const char *, void **>> *xhook_list;
map<string, vector<JNINativeMethod>, StringCmp> *jni_hook_list;
hash_map<xstring, tree_map<xstring, tree_map<xstring, void *>>> *jni_method_map;

// Current context
HookContext *g_ctx;
const JNINativeInterface *old_functions;
JNINativeInterface *new_functions;

#define HOOK_JNI(method) \
if (methods[i].name == #method##sv) { \
    for (int j = 0; j < method##_methods_num; ++j) { \
        if (strcmp(methods[i].signature, method##_methods[j].signature) == 0) { \
            jni_hook_list->try_emplace(className).first->second.push_back(methods[i]); \
            method##_orig = methods[i].fnPtr;        \
            newMethods[i] = method##_methods[j];     \
            LOGI("zygisk: replaced %s#" #method "\n", className);               \
            --hook_cnt;  \
            break;       \
        }                \
    }                    \
    continue;            \
}

// JNI method hook definitions, auto generated
#include "jni_hooks.hpp"

#undef HOOK_JNI

jclass gClassRef;
jmethodID class_getName;
string get_class_name(JNIEnv *env, jclass clazz) {
    if (!gClassRef) {
        jclass cls = env->FindClass("java/lang/Class");
        gClassRef = (jclass) env->NewGlobalRef(cls);
        env->DeleteLocalRef(cls);
        class_getName = env->GetMethodID(gClassRef, "getName", "()Ljava/lang/String;");
    }
    auto nameRef = (jstring) env->CallObjectMethod(clazz, class_getName);
    const char *name = env->GetStringUTFChars(nameRef, nullptr);
    string className(name);
    env->ReleaseStringUTFChars(nameRef, name);
    std::replace(className.begin(), className.end(), '.', '/');
    return className;
}

// -----------------------------------------------------------------

#define DCL_HOOK_FUNC(ret, func, ...) \
ret (*old_##func)(__VA_ARGS__);       \
ret new_##func(__VA_ARGS__)

jint env_RegisterNatives(
        JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint numMethods) {
    auto className = get_class_name(env, clazz);
    VLOG("zygisk: JNIEnv->RegisterNatives [%s]\n", className.data());
    auto newMethods = hookAndSaveJNIMethods(className.data(), methods, numMethods);
    return old_functions->RegisterNatives(env, clazz, newMethods.get() ?: methods, numMethods);
}

DCL_HOOK_FUNC(int, jniRegisterNativeMethods,
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {
    VLOG("zygisk: jniRegisterNativeMethods [%s]\n", className);
    auto newMethods = hookAndSaveJNIMethods(className, methods, numMethods);
    return old_jniRegisterNativeMethods(env, className, newMethods.get() ?: methods, numMethods);
}

// Skip actual fork and return cached result if applicable
DCL_HOOK_FUNC(int, fork) {
    return (g_ctx && g_ctx->pid >= 0) ? g_ctx->pid : old_fork();
}

// This is the latest point where we can still connect to the magiskd main socket
DCL_HOOK_FUNC(int, selinux_android_setcontext,
        uid_t uid, int isSystemServer, const char *seinfo, const char *pkgname) {
    if (g_ctx && g_ctx->flags[DENY_FLAG]) {
        if (remote_request_unmount() == 0) {
            LOGD("zygisk: mount namespace cleaned up\n");
        }
    }
    return old_selinux_android_setcontext(uid, isSystemServer, seinfo, pkgname);
}

// A place to clean things up before calling into zygote::ForkCommon/SpecializeCommon
DCL_HOOK_FUNC(void, android_log_close) {
    HookContext::close_fds();
    if (g_ctx && g_ctx->pid <= 0) {
        // In child process, no longer be able to access to magiskd
        android_logging();
    }
    old_android_log_close();
}

// -----------------------------------------------------------------

// The original android::AppRuntime virtual table
void **gAppRuntimeVTable;

// This method is a trampoline for hooking JNIEnv->RegisterNatives
void onVmCreated(void *self, JNIEnv* env) {
    LOGD("zygisk: AppRuntime::onVmCreated\n");

    // Restore virtual table
    auto new_table = *reinterpret_cast<void***>(self);
    *reinterpret_cast<void***>(self) = gAppRuntimeVTable;
    delete[] new_table;

    new_functions = new JNINativeInterface();
    memcpy(new_functions, env->functions, sizeof(*new_functions));
    new_functions->RegisterNatives = &env_RegisterNatives;

    // Replace the function table in JNIEnv to hook RegisterNatives
    old_functions = env->functions;
    env->functions = new_functions;
}

template<int N>
void vtable_entry(void *self, JNIEnv* env) {
    // The first invocation will be onVmCreated. It will also restore the vtable.
    onVmCreated(self, env);
    // Call original function
    reinterpret_cast<decltype(&onVmCreated)>(gAppRuntimeVTable[N])(self, env);
}

// This method is a trampoline for swizzling android::AppRuntime vtable
bool swizzled = false;
DCL_HOOK_FUNC(void, setArgv0, void *self, const char *argv0, bool setProcName) {
    if (swizzled) {
        old_setArgv0(self, argv0, setProcName);
        return;
    }

    LOGD("zygisk: AndroidRuntime::setArgv0\n");

    // We don't know which entry is onVmCreated, so overwrite every one
    // We also don't know the size of the vtable, but 8 is more than enough
    auto new_table = new void*[8];
    new_table[0] = reinterpret_cast<void*>(&vtable_entry<0>);
    new_table[1] = reinterpret_cast<void*>(&vtable_entry<1>);
    new_table[2] = reinterpret_cast<void*>(&vtable_entry<2>);
    new_table[3] = reinterpret_cast<void*>(&vtable_entry<3>);
    new_table[4] = reinterpret_cast<void*>(&vtable_entry<4>);
    new_table[5] = reinterpret_cast<void*>(&vtable_entry<5>);
    new_table[6] = reinterpret_cast<void*>(&vtable_entry<6>);
    new_table[7] = reinterpret_cast<void*>(&vtable_entry<7>);

    // Swizzle C++ vtable to hook virtual function
    gAppRuntimeVTable = *reinterpret_cast<void***>(self);
    *reinterpret_cast<void***>(self) = new_table;
    swizzled = true;

    old_setArgv0(self, argv0, setProcName);
}

#undef DCL_HOOK_FUNC

// -----------------------------------------------------------------

void HookContext::run_modules_pre()  { /* TODO */ }
void HookContext::run_modules_post() { /* TODO */ }

void HookContext::close_fds() {
    close(logd_fd.exchange(-1));
}

// -----------------------------------------------------------------

void HookContext::nativeSpecializeAppProcess_pre() {
    g_ctx = this;
    flags[APP_SPECIALIZE] = true;
    process = env->GetStringUTFChars(args->nice_name, nullptr);
    if (flags[FORK_AND_SPECIALIZE]) {
        VLOG("zygisk: pre  forkAndSpecialize [%s]\n", process);
    } else {
        VLOG("zygisk: pre  specialize [%s]\n", process);
    }

    remote_get_app_info(args->uid, process, &info);

    /* TODO: Handle MOUNT_EXTERNAL_NONE */
    if (args->mount_external != 0 && info.on_denylist) {
        LOGI("zygisk: [%s] is on the denylist\n", process);
        flags[DENY_FLAG] = true;
    } else {
        run_modules_pre();
    }
}

void HookContext::nativeSpecializeAppProcess_post() {
    if (flags[FORK_AND_SPECIALIZE]) {
        VLOG("zygisk: post forkAndSpecialize [%s]\n", process);
    } else {
        VLOG("zygisk: post specialize [%s]\n", process);
    }

    env->ReleaseStringUTFChars(args->nice_name, process);
    if (flags[DENY_FLAG]) {
        self_unload();
    } else {
        run_modules_post();
    }
    if (info.is_magisk_app) {
        setenv("ZYGISK_ENABLED", "1", 1);
    }
    g_ctx = nullptr;
}

void HookContext::nativeForkSystemServer_pre() {
    fork_pre();
    flags[SERVER_SPECIALIZE] = true;
    if (pid == 0) {
        VLOG("zygisk: pre  forkSystemServer\n");
        run_modules_pre();
        close_fds();
    }
}

void HookContext::nativeForkSystemServer_post() {
    if (pid == 0) {
        android_logging();
        VLOG("zygisk: post forkSystemServer\n");
        run_modules_post();
    }
    fork_post();
}

void HookContext::nativeForkAndSpecialize_pre() {
    fork_pre();
    flags[FORK_AND_SPECIALIZE] = true;
    if (pid == 0) {
        nativeSpecializeAppProcess_pre();
        close_fds();
    }
}

void HookContext::nativeForkAndSpecialize_post() {
    if (pid == 0) {
        android_logging();
        nativeSpecializeAppProcess_post();
    }
    fork_post();
}

int sigmask(int how, int signum) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, signum);
    return sigprocmask(how, &set, nullptr);
}

// Do our own fork before loading any 3rd party code
// First block SIGCHLD, unblock after original fork is done
void HookContext::fork_pre() {
    g_ctx = this;
    sigmask(SIG_BLOCK, SIGCHLD);
    pid = old_fork();
}

// Unblock SIGCHLD in case the original method didn't
void HookContext::fork_post() {
    sigmask(SIG_UNBLOCK, SIGCHLD);
    g_ctx = nullptr;
}

} // namespace

static bool hook_refresh() {
    if (xhook_refresh(0) == 0) {
        xhook_clear();
        LOGI("zygisk: xhook success\n");
        return true;
    } else {
        LOGE("zygisk: xhook failed\n");
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

#define XHOOK_REGISTER_SYM(PATH_REGEX, SYM, NAME) \
    hook_register(PATH_REGEX, SYM, (void*) new_##NAME, (void **) &old_##NAME)

#define XHOOK_REGISTER(PATH_REGEX, NAME) \
    XHOOK_REGISTER_SYM(PATH_REGEX, #NAME, NAME)

#define ANDROID_RUNTIME ".*/libandroid_runtime.so$"
#define APP_PROCESS     "^/system/bin/app_process.*"

void hook_functions() {
#if MAGISK_DEBUG
    xhook_enable_debug(1);
    xhook_enable_sigsegv_protection(0);
#endif
    default_new(xhook_list);
    default_new(jni_hook_list);
    default_new(jni_method_map);

    XHOOK_REGISTER(ANDROID_RUNTIME, fork);
    XHOOK_REGISTER(ANDROID_RUNTIME, selinux_android_setcontext);
    XHOOK_REGISTER(ANDROID_RUNTIME, jniRegisterNativeMethods);
    XHOOK_REGISTER_SYM(ANDROID_RUNTIME, "__android_log_close", android_log_close);
    hook_refresh();

    // Remove unhooked methods
    xhook_list->erase(
            std::remove_if(xhook_list->begin(), xhook_list->end(),
            [](auto &t) { return *std::get<2>(t) == nullptr;}),
            xhook_list->end());

    if (old_jniRegisterNativeMethods == nullptr) {
        LOGD("zygisk: jniRegisterNativeMethods not hooked, using fallback\n");

        // android::AndroidRuntime::setArgv0(const char*, bool)
        XHOOK_REGISTER_SYM(APP_PROCESS, "_ZN7android14AndroidRuntime8setArgv0EPKcb", setArgv0);
        hook_refresh();

        // We still need old_jniRegisterNativeMethods as other code uses it
        // android::AndroidRuntime::registerNativeMethods(_JNIEnv*, const char*, const JNINativeMethod*, int)
        constexpr char sig[] = "_ZN7android14AndroidRuntime21registerNativeMethodsEP7_JNIEnvPKcPK15JNINativeMethodi";
        *(void **) &old_jniRegisterNativeMethods = dlsym(RTLD_DEFAULT, sig);
    }
}

bool unhook_functions() {
    bool success = true;

    // Restore JNIEnv
    if (g_ctx->env->functions == new_functions) {
        g_ctx->env->functions = old_functions;
        if (gClassRef) {
            g_ctx->env->DeleteGlobalRef(gClassRef);
            gClassRef = nullptr;
            class_getName = nullptr;
        }
    }

    // Do NOT call the destructor
    operator delete(jni_method_map);
    // Directly unmap the whole memory block
    jni_hook::memory_block::release();

    // Unhook JNI methods
    for (const auto &[clz, methods] : *jni_hook_list) {
        if (!methods.empty() && old_jniRegisterNativeMethods(
                g_ctx->env, clz.data(), methods.data(), methods.size()) != 0) {
            LOGE("zygisk: Failed to restore JNI hook of class [%s]\n", clz.data());
            success = false;
        }
    }
    delete jni_hook_list;

    // Unhook xhook
    for (const auto &[path, sym, old_func] : *xhook_list) {
        if (xhook_register(path, sym, *old_func, nullptr) != 0) {
            LOGE("zygisk: Failed to register xhook [%s]\n", sym);
            success = false;
        }
    }
    delete xhook_list;
    if (!hook_refresh()) {
        LOGE("zygisk: Failed to restore xhook\n");
        success = false;
    }

    return success;
}
