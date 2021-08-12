#include <dlfcn.h>
#include <xhook.h>
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

struct HookContext {
    int pid;
    bool do_hide;
    union {
        SpecializeAppProcessArgs *args;
        ForkSystemServerArgs *server_args;
        void *raw_args;
    };
};

// Global variables
vector<tuple<const char *, const char *, void **>> *xhook_list;
vector<JNINativeMethod> *jni_hook_list;
hash_map<xstring, tree_map<xstring, tree_map<xstring, void *>>> *jni_method_map;

HookContext *current_ctx;
JavaVM *g_jvm;
const JNINativeInterface *old_functions;
JNINativeInterface *new_functions;

#define DCL_JNI_FUNC(name) \
void *name##_orig;         \
void name##_pre(HookContext *ctx, JNIEnv *env, jclass clazz); \
void name##_post(HookContext *ctx, JNIEnv *env, jclass clazz);

// JNI method declarations
DCL_JNI_FUNC(nativeForkAndSpecialize)
DCL_JNI_FUNC(nativeSpecializeAppProcess)
DCL_JNI_FUNC(nativeForkSystemServer)

#undef DCL_JNI_FUNC

// JNI method definitions
// Includes all method signatures of all supported Android versions
#include "jni_hooks.hpp"

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

unique_ptr<JNINativeMethod[]> hookAndSaveJNIMethods(
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {
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

#define DCL_HOOK_FUNC(ret, func, ...) \
ret (*old_##func)(__VA_ARGS__);       \
ret new_##func(__VA_ARGS__)

jint jniRegisterNatives(
        JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint numMethods) {
    auto className = get_class_name(env, clazz);
    LOGD("hook: JNIEnv->RegisterNatives %s\n", className.data());
    auto newMethods = hookAndSaveJNIMethods(env, className.data(), methods, numMethods);
    return old_functions->RegisterNatives(env, clazz, newMethods.get() ?: methods, numMethods);
}

DCL_HOOK_FUNC(int, jniRegisterNativeMethods,
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {
    LOGD("hook: jniRegisterNativeMethods %s\n", className);
    auto newMethods = hookAndSaveJNIMethods(env, className, methods, numMethods);
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

// -----------------------------------------------------------------

// The original android::AppRuntime virtual table
void **gAppRuntimeVTable;

// This method is a trampoline for hooking JNIEnv->RegisterNatives
void onVmCreated(void *self, JNIEnv* env) {
    LOGD("hook: AppRuntime::onVmCreated\n");

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

    LOGD("hook: AndroidRuntime::setArgv0\n");

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

void nativeSpecializeAppProcess_pre(HookContext *ctx, JNIEnv *env, jclass clazz) {
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

void nativeSpecializeAppProcess_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    current_ctx = nullptr;
    LOGD("hook: %s\n", __FUNCTION__);

    if (ctx->do_hide)
        self_unload();
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

void nativeForkAndSpecialize_pre(HookContext *ctx, JNIEnv *env, jclass clazz) {
    PRE_FORK();
    nativeSpecializeAppProcess_pre(ctx, env, clazz);
}

void nativeForkAndSpecialize_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    POST_FORK();
    nativeSpecializeAppProcess_post(ctx, env, clazz);
}

// -----------------------------------------------------------------

void nativeForkSystemServer_pre(HookContext *ctx, JNIEnv *env, jclass clazz) {
    if (env->functions == new_functions) {
        // Restore JNIEnv
        env->functions = old_functions;
        if (gClassRef) {
            env->DeleteGlobalRef(gClassRef);
            gClassRef = nullptr;
            class_getName = nullptr;
        }
    }

    PRE_FORK();
    LOGD("hook: %s\n", __FUNCTION__);
}

void nativeForkSystemServer_post(HookContext *ctx, JNIEnv *env, jclass clazz) {
    POST_FORK();
    LOGD("hook: %s\n", __FUNCTION__);
}

#undef PRE_FORK
#undef POST_FORK

// -----------------------------------------------------------------

bool hook_refresh() {
    if (xhook_refresh(0) == 0) {
        xhook_clear();
        LOGI("hook: xhook success\n");
        return true;
    } else {
        LOGE("hook: xhook failed\n");
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

template<class T>
static inline void default_new(T *&p) { p = new T(); }

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
        LOGD("hook: jniRegisterNativeMethods not used\n");

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
