#include <dlfcn.h>
#include <xhook.h>
#include <bitset>

#include <utils.hpp>
#include <flags.hpp>
#include <daemon.hpp>

#include "inject.hpp"
#include "memory.hpp"
#include "api.hpp"

using namespace std;
using jni_hook::hash_map;
using jni_hook::tree_map;
using xstring = jni_hook::string;

namespace {

#define DCL_JNI_FUNC(name) \
static void *name##_orig;  \
void name##_pre();         \
void name##_post();

#define DEF_STATIC(name) \
void *HookContext::name##_orig = nullptr;

enum {
    DENY_FLAG,
    APP_FORK,
    FLAG_MAX
};

struct HookContext {
    JNIEnv *env;
    jclass clazz;
    union {
        SpecializeAppProcessArgs *args;
        ForkSystemServerArgs *server_args;
        void *raw_args;
    };
    const char *process;
    int pid;
    bitset<FLAG_MAX> flags;

    HookContext() : process(nullptr), pid(0) {}

    // JNI method declarations
    DCL_JNI_FUNC(nativeForkAndSpecialize)
    DCL_JNI_FUNC(nativeSpecializeAppProcess)
    DCL_JNI_FUNC(nativeForkSystemServer)
};
DEF_STATIC(nativeForkAndSpecialize)
DEF_STATIC(nativeSpecializeAppProcess)
DEF_STATIC(nativeForkSystemServer)

#undef DCL_JNI_FUNC
#undef DEF_STATIC

// Global variables
vector<tuple<const char *, const char *, void **>> *xhook_list;
vector<JNINativeMethod> *jni_hook_list;
hash_map<xstring, tree_map<xstring, tree_map<xstring, void *>>> *jni_method_map;

// Current context
HookContext *g_ctx;
const JNINativeInterface *old_functions;
JNINativeInterface *new_functions;

// JNI method definitions
// Includes method signatures of all supported Android versions
#include "jni_hooks.hpp"

#define HOOK_JNI(method) \
if (methods[i].name == #method##sv) { \
    jni_hook_list->push_back(methods[i]);              \
    HookContext::method##_orig = methods[i].fnPtr;     \
    for (int j = 0; j < method##_methods_num; ++j) {   \
        if (strcmp(methods[i].signature, method##_methods[j].signature) == 0) { \
            newMethods[i] = method##_methods[j];       \
            LOGI("zygisk: replaced #" #method "\n");   \
            ++hooked;    \
            break;       \
        }                \
    }                    \
    continue;            \
}

unique_ptr<JNINativeMethod[]>hookAndSaveJNIMethods(
        const char *className, const JNINativeMethod *methods, int numMethods) {
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
            HOOK_JNI(nativeForkAndSpecialize)
            HOOK_JNI(nativeSpecializeAppProcess)
            HOOK_JNI(nativeForkSystemServer)
        }
    }
    return newMethods;
}

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

// TODO
int remote_check_denylist(int uid, const char *process) { return 0; }
void remote_request_unmount() {}

// -----------------------------------------------------------------

#define DCL_HOOK_FUNC(ret, func, ...) \
ret (*old_##func)(__VA_ARGS__);       \
ret new_##func(__VA_ARGS__)

jint jniRegisterNatives(
        JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint numMethods) {
    auto className = get_class_name(env, clazz);
    LOGD("zygisk: JNIEnv->RegisterNatives [%s]\n", className.data());
    auto newMethods = hookAndSaveJNIMethods(className.data(), methods, numMethods);
    return old_functions->RegisterNatives(env, clazz, newMethods.get() ?: methods, numMethods);
}

DCL_HOOK_FUNC(int, jniRegisterNativeMethods,
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {
    LOGD("zygisk: jniRegisterNativeMethods [%s]\n", className);
    auto newMethods = hookAndSaveJNIMethods(className, methods, numMethods);
    return old_jniRegisterNativeMethods(env, className, newMethods.get() ?: methods, numMethods);
}

DCL_HOOK_FUNC(int, fork) {
    return g_ctx ? g_ctx->pid : old_fork();
}

DCL_HOOK_FUNC(int, selinux_android_setcontext,
        uid_t uid, int isSystemServer, const char *seinfo, const char *pkgname) {
    if (g_ctx && g_ctx->flags[DENY_FLAG]) {
        // Ask magiskd to cleanup the mount namespace before switching context
        // This is the latest point where we can still connect to the magiskd main socket
        remote_request_unmount();
        LOGD("zygisk: process successfully hidden\n");
    }
    return old_selinux_android_setcontext(uid, isSystemServer, seinfo, pkgname);
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
    new_functions->RegisterNatives = &jniRegisterNatives;

    // Replace the function table in JNIEnv to hook RegisterNatives
    old_functions = env->functions;
    env->functions = new_functions;
}

template<int N>
void vtable_entry(void *self, JNIEnv* env) {
    // The first invocation will be onVmCreated
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

void HookContext::nativeSpecializeAppProcess_pre() {
    g_ctx = this;
    process = env->GetStringUTFChars(args->nice_name, nullptr);
    if (flags[APP_FORK]) {
        LOGD("zygisk: pre app fork [%s]\n", process);
    } else {
        LOGD("zygisk: pre specialize [%s]\n", process);
    }

    /* TODO: Handle MOUNT_EXTERNAL_NONE */
    if (args->mount_external != 0 && remote_check_denylist(args->uid, process)) {
        flags[DENY_FLAG] = true;
        LOGI("zygisk: [%s] is on the denylist\n", process);
    }
}

void HookContext::nativeSpecializeAppProcess_post() {
    if (flags[APP_FORK]) {
        LOGD("zygisk: post app fork [%s]\n", process);
    } else {
        LOGD("zygisk: post specialize [%s]\n", process);
    }

    env->ReleaseStringUTFChars(args->nice_name, process);
    if (flags[DENY_FLAG])
        self_unload();
    g_ctx = nullptr;
}

// -----------------------------------------------------------------

int sigmask(int how, int signum) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, signum);
    return sigprocmask(how, &set, nullptr);
}

// Do our own fork before loading any 3rd party code
// First block SIGCHLD, unblock after original fork is done
#define PRE_FORK() \
    g_ctx = this;  \
    sigmask(SIG_BLOCK, SIGCHLD); \
    pid = old_fork();            \
    if (pid != 0)  \
        return;

// Unblock SIGCHLD in case the original method didn't
#define POST_FORK() \
    run_finally f([]{g_ctx = nullptr;}); \
    sigmask(SIG_UNBLOCK, SIGCHLD);       \
    if (pid != 0)   \
        return;

void HookContext::nativeForkAndSpecialize_pre() {
    PRE_FORK()
    flags[APP_FORK] = true;
    nativeSpecializeAppProcess_pre();
}

void HookContext::nativeForkAndSpecialize_post() {
    POST_FORK()
    nativeSpecializeAppProcess_post();
}

// -----------------------------------------------------------------

void HookContext::nativeForkSystemServer_pre() {
    PRE_FORK()
    LOGD("zygisk: pre server fork\n");
}

void HookContext::nativeForkSystemServer_post() {
    POST_FORK()
    LOGD("zygisk: post server fork\n");
}

#undef PRE_FORK
#undef POST_FORK

// -----------------------------------------------------------------

bool hook_refresh() {
    if (xhook_refresh(0) == 0) {
        xhook_clear();
        LOGI("zygisk: xhook success\n");
        return true;
    } else {
        LOGE("zygisk: xhook failed\n");
        return false;
    }
}

int hook_register(const char *path, const char *symbol, void *new_func, void **old_func) {
    int ret = xhook_register(path, symbol, new_func, old_func);
    if (ret != 0) {
        LOGE("hook: Failed to register hook \"%s\"\n", symbol);
        return ret;
    }
    xhook_list->emplace_back(path, symbol, old_func);
    return 0;
}

} // namespace

#define XHOOK_REGISTER_SYM(PATH_REGEX, SYM, NAME) \
    hook_register(PATH_REGEX, SYM, (void*) new_##NAME, (void **) &old_##NAME)

#define XHOOK_REGISTER(PATH_REGEX, NAME) \
    XHOOK_REGISTER_SYM(PATH_REGEX, #NAME, NAME)

#define ANDROID_RUNTIME ".*/libandroid_runtime.so$"
#define APP_PROCESS "^/system/bin/app_process.*"

void hook_functions() {
#ifdef MAGISK_DEBUG
    xhook_enable_debug(1);
    xhook_enable_sigsegv_protection(0);
#endif
    default_new(xhook_list);
    default_new(jni_hook_list);
    default_new(jni_method_map);

    XHOOK_REGISTER(ANDROID_RUNTIME, fork);
    XHOOK_REGISTER(ANDROID_RUNTIME, selinux_android_setcontext);
    XHOOK_REGISTER(ANDROID_RUNTIME, jniRegisterNativeMethods);
    hook_refresh();

    // Remove unhooked methods
    xhook_list->erase(
            std::remove_if(xhook_list->begin(), xhook_list->end(),
            [](auto &t) { return *std::get<2>(t) == nullptr;}),
            xhook_list->end());

    if (old_jniRegisterNativeMethods == nullptr) {
        LOGD("zygisk: jniRegisterNativeMethods not hooked, using fallback\n");

        // android::AndroidRuntime::setArgv0(const char *, bool)
        XHOOK_REGISTER_SYM(APP_PROCESS, "_ZN7android14AndroidRuntime8setArgv0EPKcb", setArgv0);
        hook_refresh();

        // We still need old_jniRegisterNativeMethods as other code uses it
        // android::AndroidRuntime::registerNativeMethods(_JNIEnv*, const char *, const JNINativeMethod *, int)
        constexpr char sig[] = "_ZN7android14AndroidRuntime21registerNativeMethodsEP7_JNIEnvPKcPK15JNINativeMethodi";
        *(void **) &old_jniRegisterNativeMethods = dlsym(RTLD_DEFAULT, sig);
    }
}

bool unhook_functions() {
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
    if (!jni_hook_list->empty() && old_jniRegisterNativeMethods(g_ctx->env,
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
