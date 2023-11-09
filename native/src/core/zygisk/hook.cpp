#include <sys/mount.h>
#include <dlfcn.h>
#include <unwind.h>

#include <lsplt.hpp>

#include <base.hpp>
#include <consts.hpp>

#include "zygisk.hpp"
#include "module.hpp"
#include "jni_hooks.hpp"

using namespace std;

static void hook_unloader();
static void unhook_functions();
static void hook_jni_env();
static void post_native_bridge_load();

struct HookState {
    vector<tuple<dev_t, ino_t, const char *, void **>> plt_backup;
    map<string, vector<JNINativeMethod>, StringCmp> jni_backup;
    JNINativeInterface new_env{};
    const JNINativeInterface *old_env = nullptr;
    const NativeBridgeRuntimeCallbacks *runtime_callbacks = nullptr;
};

// Global contexts
HookContext *g_ctx;
static HookState *g_state;
static bool should_unmap_zygisk = false;

// -----------------------------------------------------------------

#define DCL_HOOK_FUNC(ret, func, ...) \
ret (*old_##func)(__VA_ARGS__);       \
ret new_##func(__VA_ARGS__)

DCL_HOOK_FUNC(static void, androidSetCreateThreadFunc, void *func) {
    ZLOGD("androidSetCreateThreadFunc\n");
    hook_jni_env();
    old_androidSetCreateThreadFunc(func);
}

// Skip actual fork and return cached result if applicable
DCL_HOOK_FUNC(int, fork) {
    return (g_ctx && g_ctx->pid >= 0) ? g_ctx->pid : old_fork();
}

// Unmount stuffs in the process's private mount namespace
DCL_HOOK_FUNC(static int, unshare, int flags) {
    int res = old_unshare(flags);
    if (g_ctx && (flags & CLONE_NEWNS) != 0 && res == 0 &&
        // For some unknown reason, unmounting app_process in SysUI can break.
        // This is reproducible on the official AVD running API 26 and 27.
        // Simply avoid doing any unmounts for SysUI to avoid potential issues.
        (g_ctx->info_flags & PROCESS_IS_SYS_UI) == 0) {
        if (g_ctx->flags & DO_REVERT_UNMOUNT) {
            revert_unmount();
        }
        // Restore errno back to 0
        errno = 0;
    }
    return res;
}

// This is the last moment before the secontext of the process changes
DCL_HOOK_FUNC(static int, selinux_android_setcontext,
              uid_t uid, bool isSystemServer, const char *seinfo, const char *pkgname) {
    // Pre-fetch logd before secontext transition
    zygisk_get_logd();
    return old_selinux_android_setcontext(uid, isSystemServer, seinfo, pkgname);
}

// Close file descriptors to prevent crashing
DCL_HOOK_FUNC(static void, android_log_close) {
    if (g_ctx == nullptr || !(g_ctx->flags & SKIP_CLOSE_LOG_PIPE)) {
        // This happens during forks like nativeForkApp, nativeForkUsap,
        // nativeForkSystemServer, and nativeForkAndSpecialize.
        zygisk_close_logd();
    }
    old_android_log_close();
}

// We cannot directly call `dlclose` to unload ourselves, otherwise when `dlclose` returns,
// it will return to our code which has been unmapped, causing segmentation fault.
// Instead, we hook `pthread_attr_destroy` which will be called when VM daemon threads start.
DCL_HOOK_FUNC(static int, pthread_attr_destroy, void *target) {
    int res = old_pthread_attr_destroy((pthread_attr_t *)target);

    // Only perform unloading on the main thread
    if (gettid() != getpid())
        return res;

    ZLOGV("pthread_attr_destroy\n");
    if (should_unmap_zygisk) {
        unhook_functions();
        if (should_unmap_zygisk) {
            delete g_state;

            // Because both `pthread_attr_destroy` and `dlclose` have the same function signature,
            // we can use `musttail` to let the compiler reuse our stack frame and thus
            // `dlclose` will directly return to the caller of `pthread_attr_destroy`.
            [[clang::musttail]] return dlclose(self_handle);
        }
    }

    delete g_state;
    return res;
}

// it should be safe to assume all dlclose's in libnativebridge are for zygisk_loader
DCL_HOOK_FUNC(static int, dlclose, void *handle) {
    static bool kDone = false;
    if (!kDone) {
        ZLOGV("dlclose zygisk_loader\n");
        kDone = true;
        post_native_bridge_load();
    }
    [[clang::musttail]] return old_dlclose(handle);
}

#undef DCL_HOOK_FUNC

// -----------------------------------------------------------------

HookContext::HookContext(JNIEnv *env, void *args) :
    env(env), args{args}, process(nullptr), pid(-1), flags(0), info_flags(0),
    hook_info_lock(PTHREAD_MUTEX_INITIALIZER) { g_ctx = this; }

HookContext::~HookContext() {
    // This global pointer points to a variable on the stack.
    // Set this to nullptr to prevent leaking local variable.
    // This also disables most plt hooked functions.
    g_ctx = nullptr;

    if (!is_child())
        return;

    zygisk_close_logd();
    android_logging();

    should_unmap_zygisk = true;

    // Unhook JNI methods
    for (const auto &[clz, methods] : g_state->jni_backup) {
        if (!methods.empty() && env->RegisterNatives(
                env->FindClass(clz.data()), methods.data(),
                static_cast<int>(methods.size())) != 0) {
            ZLOGE("Failed to restore JNI hook of class [%s]\n", clz.data());
            should_unmap_zygisk = false;
        }
    }

    // Strip out all API function pointers
    for (auto &m : modules) {
        m.clearApi();
    }

    hook_unloader();
}

// -----------------------------------------------------------------

inline void *unwind_get_region_start(_Unwind_Context *ctx) {
    auto fp = _Unwind_GetRegionStart(ctx);
#if defined(__arm__)
    // On arm32, we need to check if the pc is in thumb mode,
    // if so, we need to set the lowest bit of fp to 1
    auto pc = _Unwind_GetGR(ctx, 15); // r15 is pc
    if (pc & 1) {
        // Thumb mode
        fp |= 1;
    }
#endif
    return reinterpret_cast<void *>(fp);
}

// As we use NativeBridgeRuntimeCallbacks to reload native bridge and to hook jni functions,
// we need to find it by the native bridge's unwind context.
// For abis that use registers to pass arguments, i.e. arm32, arm64, x86_64, the registers are
// caller-saved, and they are not preserved in the unwind context. However, they will be saved
// into the callee-saved registers, so we will search the callee-saved registers for the second
// argument, which is the pointer to NativeBridgeRuntimeCallbacks.
// For x86, whose abi uses stack to pass arguments, we can directly get the pointer to
// NativeBridgeRuntimeCallbacks from the stack.
static const NativeBridgeRuntimeCallbacks* find_runtime_callbacks(struct _Unwind_Context *ctx) {
    // Find the writable memory region of libart.so, where the NativeBridgeRuntimeCallbacks is located.
    auto [start, end] = []()-> tuple<uintptr_t, uintptr_t> {
        for (const auto &map : lsplt::MapInfo::Scan()) {
            if (map.path.ends_with("/libart.so") && map.perms == (PROT_WRITE | PROT_READ)) {
                ZLOGV("libart.so: start=%p, end=%p\n",
                      reinterpret_cast<void *>(map.start), reinterpret_cast<void *>(map.end));
                return {map.start, map.end};
            }
        }
        return {0, 0};
    }();
#if defined(__aarch64__)
    // r19-r28 are callee-saved registers
    for (int i = 19; i <= 28; ++i) {
        auto val = static_cast<uintptr_t>(_Unwind_GetGR(ctx, i));
        ZLOGV("r%d = %p\n", i, reinterpret_cast<void *>(val));
        if (val >= start && val < end)
            return reinterpret_cast<const NativeBridgeRuntimeCallbacks*>(val);
    }
#elif defined(__arm__)
    // r4-r10 are callee-saved registers
    for (int i = 4; i <= 10; ++i) {
        auto val = static_cast<uintptr_t>(_Unwind_GetGR(ctx, i));
        ZLOGV("r%d = %p\n", i, reinterpret_cast<void *>(val));
        if (val >= start && val < end)
            return reinterpret_cast<const NativeBridgeRuntimeCallbacks*>(val);
    }
#elif defined(__i386__)
    // get ebp, which points to the bottom of the stack frame
    auto ebp = static_cast<uintptr_t>(_Unwind_GetGR(ctx, 5));
    // 1 pointer size above ebp is the old ebp
    // 2 pointer sizes above ebp is the return address
    // 3 pointer sizes above ebp is the 2nd arg
    auto val = *reinterpret_cast<uintptr_t *>(ebp + 3 * sizeof(void *));
    ZLOGV("ebp + 3 * ptr_size = %p\n", reinterpret_cast<void *>(val));
    if (val >= start && val < end)
        return reinterpret_cast<const NativeBridgeRuntimeCallbacks*>(val);
#elif defined(__x86_64__)
    // r12-r15 and rbx are callee-saved registers, but the compiler is likely to use them reversely
    for (int i : {3, 15, 14, 13, 12}) {
        auto val = static_cast<uintptr_t>(_Unwind_GetGR(ctx, i));
        ZLOGV("r%d = %p\n", i, reinterpret_cast<void *>(val));
        if (val >= start && val < end)
            return reinterpret_cast<const NativeBridgeRuntimeCallbacks*>(val);
    }
#else
#error "Unsupported architecture"
#endif
    return nullptr;
}

static void post_native_bridge_load() {
    // Unwind to find the address of android::LoadNativeBridge and NativeBridgeRuntimeCallbacks
    bool (*load_native_bridge)(const char *nb_library_filename,
            const NativeBridgeRuntimeCallbacks *runtime_cbs) = nullptr;
    _Unwind_Backtrace(+[](struct _Unwind_Context *ctx, void *arg) -> _Unwind_Reason_Code {
        void *fp = unwind_get_region_start(ctx);
        Dl_info info{};
        dladdr(fp, &info);
        ZLOGV("backtrace: %p %s\n", fp, info.dli_fname ? info.dli_fname : "???");
        if (info.dli_fname && std::string_view(info.dli_fname).ends_with("/libnativebridge.so")) {
            *reinterpret_cast<void **>(arg) = fp;
            g_state->runtime_callbacks = find_runtime_callbacks(ctx);
            ZLOGV("NativeBridgeRuntimeCallbacks: %p\n", g_state->runtime_callbacks);
            return _URC_END_OF_STACK;
        }
        return _URC_NO_REASON;
    }, &load_native_bridge);

    // Reload the real native bridge if necessary
    auto nb = get_prop(NBPROP);
    auto len = sizeof(ZYGISKLDR) - 1;
    if (nb.size() > len) {
        load_native_bridge(nb.data() + len, g_state->runtime_callbacks);
    }
}

static void hook_register(dev_t dev, ino_t inode, const char *symbol, void *new_func, void **old_func) {
    if (!lsplt::RegisterHook(dev, inode, symbol, new_func, old_func)) {
        ZLOGE("Failed to register plt_hook \"%s\"\n", symbol);
        return;
    }
    g_state->plt_backup.emplace_back(dev, inode, symbol, old_func);
}

static void hook_commit() {
    if (!lsplt::CommitHook())
        ZLOGE("plt_hook failed\n");
}

#define PLT_HOOK_REGISTER_SYM(DEV, INODE, SYM, NAME) \
    hook_register(DEV, INODE, SYM, \
    reinterpret_cast<void *>(new_##NAME), reinterpret_cast<void **>(&old_##NAME))

#define PLT_HOOK_REGISTER(DEV, INODE, NAME) \
    PLT_HOOK_REGISTER_SYM(DEV, INODE, #NAME, NAME)

void hook_functions() {
    default_new(g_state);

    ino_t android_runtime_inode = 0;
    dev_t android_runtime_dev = 0;
    ino_t native_bridge_inode = 0;
    dev_t native_bridge_dev = 0;

    for (auto &map : lsplt::MapInfo::Scan()) {
        if (map.path.ends_with("/libandroid_runtime.so")) {
            android_runtime_inode = map.inode;
            android_runtime_dev = map.dev;
        } else if (map.path.ends_with("/libnativebridge.so")) {
            native_bridge_inode = map.inode;
            native_bridge_dev = map.dev;
        }
    }

    PLT_HOOK_REGISTER(native_bridge_dev, native_bridge_inode, dlclose);
    PLT_HOOK_REGISTER(android_runtime_dev, android_runtime_inode, fork);
    PLT_HOOK_REGISTER(android_runtime_dev, android_runtime_inode, unshare);
    PLT_HOOK_REGISTER(android_runtime_dev, android_runtime_inode, androidSetCreateThreadFunc);
    PLT_HOOK_REGISTER(android_runtime_dev, android_runtime_inode, selinux_android_setcontext);
    PLT_HOOK_REGISTER_SYM(android_runtime_dev, android_runtime_inode, "__android_log_close", android_log_close);
    hook_commit();

    // Remove unhooked methods
    g_state->plt_backup.erase(
            std::remove_if(g_state->plt_backup.begin(), g_state->plt_backup.end(),
            [](auto &t) { return *std::get<3>(t) == nullptr;}),
            g_state->plt_backup.end());
}

static void hook_unloader() {
    ino_t art_inode = 0;
    dev_t art_dev = 0;

    for (auto &map : lsplt::MapInfo::Scan()) {
        if (map.path.ends_with("/libart.so")) {
            art_inode = map.inode;
            art_dev = map.dev;
            break;
        }
    }

    PLT_HOOK_REGISTER(art_dev, art_inode, pthread_attr_destroy);
    hook_commit();
}

static void unhook_functions() {
    // Unhook plt_hook
    for (const auto &[dev, inode, sym, old_func] : g_state->plt_backup) {
        if (!lsplt::RegisterHook(dev, inode, sym, *old_func, nullptr)) {
            ZLOGE("Failed to register plt_hook [%s]\n", sym);
            should_unmap_zygisk = false;
        }
    }
    if (!lsplt::CommitHook()) {
        ZLOGE("Failed to restore plt_hook\n");
        should_unmap_zygisk = false;
    }
}

// -----------------------------------------------------------------

void hookJniNativeMethods(JNIEnv *env, const char *clz, JNINativeMethod *methods, int numMethods) {
    jclass clazz;
    if (!g_state || !g_state->runtime_callbacks || !env || !clz || !(clazz = env->FindClass(clz))) {
        for (auto i = 0; i < numMethods; ++i) {
            methods[i].fnPtr = nullptr;
        }
        return;
    }

    // Backup existing methods
    auto total = g_state->runtime_callbacks->getNativeMethodCount(env, clazz);
    vector<JNINativeMethod> old_methods(total);
    g_state->runtime_callbacks->getNativeMethods(env, clazz, old_methods.data(), total);

    // Replace the method
    for (auto i = 0; i < numMethods; ++i) {
        auto &method = methods[i];
        auto res = env->RegisterNatives(clazz, &method, 1);
        // It's normal that the method is not found
        if (res == JNI_ERR || env->ExceptionCheck()) {
            auto exception = env->ExceptionOccurred();
            if (exception) env->DeleteLocalRef(exception);
            env->ExceptionClear();
            method.fnPtr = nullptr;
        } else {
            // Find the old function pointer and return to caller
            for (const auto &old_method : old_methods) {
                if (strcmp(method.name, old_method.name) == 0 &&
                    strcmp(method.signature, old_method.signature) == 0) {
                    ZLOGD("replace %s#%s%s %p -> %p\n", clz,
                          method.name, method.signature, old_method.fnPtr, method.fnPtr);
                    method.fnPtr = old_method.fnPtr;
                }
            }
        }
    }
}

static string get_class_name(JNIEnv *env, jclass clazz) {
    static auto class_getName = env->GetMethodID(
            env->FindClass("java/lang/Class"), "getName", "()Ljava/lang/String;");
    auto nameRef = (jstring) env->CallObjectMethod(clazz, class_getName);
    const char *name = env->GetStringUTFChars(nameRef, nullptr);
    string className(name);
    env->ReleaseStringUTFChars(nameRef, name);
    std::replace(className.begin(), className.end(), '.', '/');
    return className;
}

static void replace_jni_methods(
        vector<JNINativeMethod> &methods, vector<JNINativeMethod> &backup,
        const JNINativeMethod *hook_methods, size_t hook_methods_size,
        void **orig_function) {
    for (auto &method : methods) {
        if (strcmp(method.name, hook_methods[0].name) == 0) {
            for (auto i = 0; i < hook_methods_size; ++i) {
                const auto &hook = hook_methods[i];
                if (strcmp(method.signature, hook.signature) == 0) {
                    backup.push_back(method);
                    *orig_function = method.fnPtr;
                    method.fnPtr = hook.fnPtr;
                    ZLOGI("replace %s\n", method.name);
                    return;
                }
            }
            ZLOGE("unknown signature of %s%s\n", method.name, method.signature);
        }
    }
}

#define HOOK_JNI(method) \
replace_jni_methods(newMethods, backup, method##_methods.data(), method##_methods.size(), &method##_orig)

static jint env_RegisterNatives(
        JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint numMethods) {
    auto className = get_class_name(env, clazz);
    if (className == "com/android/internal/os/Zygote") {
        // Restore JNIEnv as we no longer need to replace anything
        env->functions = g_state->old_env;

        vector<JNINativeMethod> newMethods(methods, methods + numMethods);
        vector<JNINativeMethod> &backup = g_state->jni_backup[className];
        HOOK_JNI(nativeForkAndSpecialize);
        HOOK_JNI(nativeSpecializeAppProcess);
        HOOK_JNI(nativeForkSystemServer);
        return g_state->old_env->RegisterNatives(env, clazz, newMethods.data(), numMethods);
    } else {
        return g_state->old_env->RegisterNatives(env, clazz, methods, numMethods);
    }
}

static void hook_jni_env() {
    using method_sig = jint(*)(JavaVM **, jsize, jsize *);
    auto get_created_vms = reinterpret_cast<method_sig>(
            dlsym(RTLD_DEFAULT, "JNI_GetCreatedJavaVMs"));
    if (!get_created_vms) {
        for (auto &map: lsplt::MapInfo::Scan()) {
            if (!map.path.ends_with("/libnativehelper.so")) continue;
            void *h = dlopen(map.path.data(), RTLD_LAZY);
            if (!h) {
                ZLOGW("Cannot dlopen libnativehelper.so: %s\n", dlerror());
                break;
            }
            get_created_vms = reinterpret_cast<method_sig>(dlsym(h, "JNI_GetCreatedJavaVMs"));
            dlclose(h);
            break;
        }
        if (!get_created_vms) {
            ZLOGW("JNI_GetCreatedJavaVMs not found\n");
            return;
        }
    }

    JavaVM *vm = nullptr;
    jsize num = 0;
    jint res = get_created_vms(&vm, 1, &num);
    if (res != JNI_OK || vm == nullptr) {
        ZLOGW("JavaVM not found\n");
        return;
    }
    JNIEnv *env = nullptr;
    res = vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (res != JNI_OK || env == nullptr) {
        ZLOGW("JNIEnv not found\n");
        return;
    }

    // Replace the function table in JNIEnv to hook RegisterNatives
    memcpy(&g_state->new_env, env->functions, sizeof(*env->functions));
    g_state->new_env.RegisterNatives = &env_RegisterNatives;
    g_state->old_env = env->functions;
    env->functions = &g_state->new_env;
}
