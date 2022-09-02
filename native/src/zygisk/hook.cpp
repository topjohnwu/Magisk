#include <android/dlext.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include <bitset>

#include <xhook.h>

#include <base.hpp>
#include <flags.h>
#include <daemon.hpp>

#include "zygisk.hpp"
#include "memory.hpp"
#include "module.hpp"
#include "deny/deny.hpp"

using namespace std;
using jni_hook::hash_map;
using jni_hook::tree_map;
using xstring = jni_hook::string;

// Extreme verbose logging
//#define ZLOGV(...) ZLOGD(__VA_ARGS__)
#define ZLOGV(...)

static bool unhook_functions();

namespace {

enum {
    APP_FORK_AND_SPECIALIZE,
    APP_SPECIALIZE,
    SERVER_FORK_AND_SPECIALIZE,
    DO_REVERT_UNMOUNT,
    CAN_UNLOAD_ZYGISK,

    FLAG_MAX
};

#define DCL_PRE_POST(name) \
void name##_pre();         \
void name##_post();

struct HookContext {
    JNIEnv *env;
    union {
        void *ptr;
        AppSpecializeArgs_v3 *app;
        ServerSpecializeArgs_v1 *server;
    } args;

    const char *process;
    vector<ZygiskModule> modules;

    int pid;
    bitset<FLAG_MAX> flags;
    uint32_t info_flags;

    HookContext() : env(nullptr), args{nullptr}, process(nullptr), pid(-1), info_flags(0) {}

    static void pre_specialize_start(DIR *dir, bitset<1024> &allowed_fds);
    void pre_specialize_end(DIR *dir, bitset<1024> &allowed_fds);
    bool setup_fd_ignore(bitset<1024> *allowed_fds = nullptr);

    void run_modules_pre(const vector<int> &fds);
    void run_modules_post();
    DCL_PRE_POST(fork)
    DCL_PRE_POST(nativeForkAndSpecialize)
    DCL_PRE_POST(nativeSpecializeAppProcess)
    DCL_PRE_POST(nativeForkSystemServer)

    void unload_zygisk();
};

#undef DCL_PRE_POST

// Global variables
vector<tuple<const char *, const char *, void **>> *xhook_list;
map<string, vector<JNINativeMethod>, StringCmp> *jni_hook_list;
hash_map<xstring, tree_map<xstring, tree_map<xstring, void *>>> *jni_method_map;

// Current context
HookContext *g_ctx;
const JNINativeInterface *old_functions;
JNINativeInterface *new_functions;

} // namespace

#define HOOK_JNI(method)                                                                     \
if (methods[i].name == #method##sv) {                                                        \
    int j = 0;                                                                               \
    for (; j < method##_methods_num; ++j) {                                                  \
        if (strcmp(methods[i].signature, method##_methods[j].signature) == 0) {              \
            jni_hook_list->try_emplace(className).first->second.push_back(methods[i]);       \
            method##_orig = methods[i].fnPtr;                                                \
            newMethods[i] = method##_methods[j];                                             \
            ZLOGI("replaced %s#" #method "\n", className);                                   \
            --hook_cnt;                                                                      \
            break;                                                                           \
        }                                                                                    \
    }                                                                                        \
    if (j == method##_methods_num) {                                                         \
        ZLOGE("unknown signature of %s#" #method ": %s\n", className, methods[i].signature); \
    }                                                                                        \
    continue;                                                                                \
}

// JNI method hook definitions, auto generated
#include "jni_hooks.hpp"

#undef HOOK_JNI

namespace {

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

#define DCL_HOOK_FUNC(ret, func, ...) \
ret (*old_##func)(__VA_ARGS__);       \
ret new_##func(__VA_ARGS__)

jint env_RegisterNatives(
        JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint numMethods) {
    auto className = get_class_name(env, clazz);
    ZLOGV("JNIEnv->RegisterNatives [%s]\n", className.data());
    auto newMethods = hookAndSaveJNIMethods(className.data(), methods, numMethods);
    return old_functions->RegisterNatives(env, clazz, newMethods.get() ?: methods, numMethods);
}

DCL_HOOK_FUNC(int, jniRegisterNativeMethods,
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {
    ZLOGV("jniRegisterNativeMethods [%s]\n", className);
    auto newMethods = hookAndSaveJNIMethods(className, methods, numMethods);
    return old_jniRegisterNativeMethods(env, className, newMethods.get() ?: methods, numMethods);
}

// Skip actual fork and return cached result if applicable
// Also unload first stage zygisk if necessary
DCL_HOOK_FUNC(int, fork) {
    return (g_ctx && g_ctx->pid >= 0) ? g_ctx->pid : old_fork();
}

// Unmount stuffs in the process's private mount namespace
DCL_HOOK_FUNC(int, unshare, int flags) {
    int res = old_unshare(flags);
    if (g_ctx && (flags & CLONE_NEWNS) != 0 && res == 0 &&
        // For some unknown reason, unmounting app_process in SysUI can break.
        // This is reproducible on the official AVD running API 26 and 27.
        // Simply avoid doing any unmounts for SysUI to avoid potential issues.
        g_ctx->process && g_ctx->process != "com.android.systemui"sv) {
        if (g_ctx->flags[DO_REVERT_UNMOUNT]) {
            revert_unmount();
        } else {
            umount2("/system/bin/app_process64", MNT_DETACH);
            umount2("/system/bin/app_process32", MNT_DETACH);
        }
        // Restore errno back to 0
        errno = 0;
    }
    return res;
}

// A place to clean things up before zygote evaluates fd table
DCL_HOOK_FUNC(void, android_log_close) {
    if (g_ctx == nullptr || !g_ctx->flags[APP_SPECIALIZE]) {
        // Close fd to prevent crashing.
        // For more info, check comments in zygisk_log_write.
        close(logd_fd.exchange(-1));
    }
    old_android_log_close();
}

// Last point before process secontext changes
DCL_HOOK_FUNC(int, selinux_android_setcontext,
        uid_t uid, int isSystemServer, const char *seinfo, const char *pkgname) {
    if (g_ctx) {
        g_ctx->flags[CAN_UNLOAD_ZYGISK] = unhook_functions();
    }
    return old_selinux_android_setcontext(uid, isSystemServer, seinfo, pkgname);
}

// -----------------------------------------------------------------

// The original android::AppRuntime virtual table
void **gAppRuntimeVTable;

// This method is a trampoline for hooking JNIEnv->RegisterNatives
void onVmCreated(void *self, JNIEnv* env) {
    ZLOGD("AppRuntime::onVmCreated\n");

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

    ZLOGD("AndroidRuntime::setArgv0\n");

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

void hookJniNativeMethods(JNIEnv *env, const char *clz, JNINativeMethod *methods, int numMethods) {
    auto class_map = jni_method_map->find(clz);
    if (class_map == jni_method_map->end()) {
        for (int i = 0; i < numMethods; ++i) {
            methods[i].fnPtr = nullptr;
        }
        return;
    }

    vector<JNINativeMethod> hooks;
    for (int i = 0; i < numMethods; ++i) {
        auto method_map = class_map->second.find(methods[i].name);
        if (method_map != class_map->second.end()) {
            auto it = method_map->second.find(methods[i].signature);
            if (it != method_map->second.end()) {
                // Copy the JNINativeMethod
                hooks.push_back(methods[i]);
                // Save the original function pointer
                methods[i].fnPtr = it->second;
                // Do not allow double hook, remove method from map
                method_map->second.erase(it);
                continue;
            }
        }
        // No matching method found, set fnPtr to null
        methods[i].fnPtr = nullptr;
    }

    if (hooks.empty())
        return;

    old_jniRegisterNativeMethods(env, clz, hooks.data(), hooks.size());
}

ZygiskModule::ZygiskModule(int id, void *handle, void *entry)
: id(id), handle(handle), entry{entry}, api{}, mod{nullptr} {
    // Make sure all pointers are null
    memset(&api, 0, sizeof(api));
    api.base.impl = this;
    api.base.registerModule = &ZygiskModule::RegisterModuleImpl;
}

bool ZygiskModule::RegisterModuleImpl(api_abi_base *api, long *module) {
    long api_version = *module;
    // Unsupported version
    if (api_version > ZYGISK_API_VERSION)
        return false;

    // Set the actual module_abi*
    api->impl->mod = { module };

    // Fill in API accordingly with module API version
    switch (api_version) {
    case 3:
    case 2: {
        auto v2 = static_cast<api_abi_v2 *>(api);
        v2->getModuleDir = [](ZygiskModule *m) { return m->getModuleDir(); };
        v2->getFlags = [](auto) { return ZygiskModule::getFlags(); };
    }
        // fallthrough
    case 1: {
        auto v1 = static_cast<api_abi_v1 *>(api);
        v1->hookJniNativeMethods = &hookJniNativeMethods;
        v1->pltHookRegister = [](const char *p, const char *s, void *n, void **o) {
            xhook_register(p, s, n, o);
        };
        v1->pltHookExclude = [](const char *p, const char *s) {
            xhook_ignore(p, s);
        };
        v1->pltHookCommit = [] {
            bool r = xhook_refresh(0) == 0;
            xhook_clear();
            return r;
        };
        v1->connectCompanion = [](ZygiskModule *m) { return m->connectCompanion(); };
        v1->setOption = [](ZygiskModule *m, auto opt) { m->setOption(opt); };
        break;
    }
    default:
        // Unknown version number
        return false;
    }

    return true;
}

int ZygiskModule::connectCompanion() const {
    if (int fd = zygisk_request(ZygiskRequest::CONNECT_COMPANION); fd >= 0) {
        write_int(fd, id);
        return fd;
    }
    return -1;
}

int ZygiskModule::getModuleDir() const {
    if (int fd = zygisk_request(ZygiskRequest::GET_MODDIR); fd >= 0) {
        write_int(fd, id);
        int dfd = recv_fd(fd);
        close(fd);
        return dfd;
    }
    return -1;
}

void ZygiskModule::setOption(zygisk::Option opt) {
    if (g_ctx == nullptr)
        return;
    switch (opt) {
    case zygisk::FORCE_DENYLIST_UNMOUNT:
        g_ctx->flags[DO_REVERT_UNMOUNT] = true;
        break;
    case zygisk::DLCLOSE_MODULE_LIBRARY:
        unload = true;
        break;
    }
}

uint32_t ZygiskModule::getFlags() {
    return g_ctx ? (g_ctx->info_flags & ~PRIVATE_MASK) : 0;
}

// -----------------------------------------------------------------

bool HookContext::setup_fd_ignore(bitset<1024> *allowed_fds) {
    if (flags[APP_SPECIALIZE]) {
        if (args.app->fds_to_ignore == nullptr) {
            // The field fds_to_ignore don't exist if
            // - Running before Android 8.0
            // - Called by nativeSpecializeAppProcess
            // In either case, FDs are not checked, so we can simply skip
            return true;
        }
        bool success = false;
        if (jintArray fdsToIgnore = *args.app->fds_to_ignore) {
            int *arr = env->GetIntArrayElements(fdsToIgnore, nullptr);
            int len = env->GetArrayLength(fdsToIgnore);
            if (allowed_fds) {
                for (int i = 0; i < len; ++i) {
                    int fd = arr[i];
                    if (fd >= 0 && fd < 1024) {
                        (*allowed_fds)[fd] = true;
                    }
                }
            }

            jintArray newFdList = nullptr;
            if (logd_fd >= 0 && (newFdList = env->NewIntArray(len + 1))) {
                success = true;
                env->SetIntArrayRegion(newFdList, 0, len, arr);
                int fd = logd_fd;
                env->SetIntArrayRegion(newFdList, len, 1, &fd);
                *args.app->fds_to_ignore = newFdList;
            }
            env->ReleaseIntArrayElements(fdsToIgnore, arr, JNI_ABORT);
        } else {
            jintArray newFdList = nullptr;
            if (logd_fd >= 0 && (newFdList = env->NewIntArray(1))) {
                success = true;
                int fd = logd_fd;
                env->SetIntArrayRegion(newFdList, 0, 1, &fd);
                *args.app->fds_to_ignore = newFdList;
            }
        }
        return success;
    }
    return false;
}

void HookContext::pre_specialize_start(DIR *dir, bitset<1024> &allowed_fds) {
    // Record all open fds
    for (dirent *entry; (entry = xreaddir(dir));) {
        int fd = parse_int(entry->d_name);
        if (fd < 0 || fd >= 1024) {
            close(fd);
            continue;
        }
        allowed_fds[fd] = true;
    }
}

void HookContext::pre_specialize_end(DIR *dir, bitset<1024> &allowed_fds) {
    bool disable_zygisk_logging = !setup_fd_ignore(&allowed_fds);

    // Close all forbidden fds to prevent crashing
    rewinddir(dir);
    for (dirent *entry; (entry = xreaddir(dir));) {
        int fd = parse_int(entry->d_name);
        if (fd < 0 || fd >= 1024 || !allowed_fds[fd]) {
            close(fd);
        }
    }

    if (disable_zygisk_logging) {
        close(logd_fd.exchange(-1));
        android_logging();
    }
}

void HookContext::run_modules_pre(const vector<int> &fds) {
    // Because the data structure stored in the vector is self referencing, in order to prevent
    // dangling pointers, the vector has to be pre-allocated to ensure reallocation does not occur
    modules.reserve(fds.size());

    for (int i = 0; i < fds.size(); ++i) {
        struct stat s{};
        if (fstat(fds[i], &s) != 0 || !S_ISREG(s.st_mode)) {
            close(fds[i]);
            continue;
        }
        android_dlextinfo info {
            .flags = ANDROID_DLEXT_USE_LIBRARY_FD,
            .library_fd = fds[i],
        };
        if (void *h = android_dlopen_ext("/jit-cache", RTLD_LAZY, &info)) {
            if (void *e = dlsym(h, "zygisk_module_entry")) {
                modules.emplace_back(i, h, e);
            }
        } else if (g_ctx->flags[SERVER_FORK_AND_SPECIALIZE]) {
            LOGW("Failed to dlopen zygisk module: %s\n", dlerror());
        }
        close(fds[i]);
    }

    for (auto &m : modules) {
        m.onLoad(env);
        if (flags[APP_SPECIALIZE]) {
            m.preAppSpecialize(args.app);
        } else if (flags[SERVER_FORK_AND_SPECIALIZE]) {
            m.preServerSpecialize(args.server);
        }
    }
}

void HookContext::run_modules_post() {
    for (const auto &m : modules) {
        if (flags[APP_SPECIALIZE]) {
            m.postAppSpecialize(args.app);
        } else if (flags[SERVER_FORK_AND_SPECIALIZE]) {
            m.postServerSpecialize(args.server);
        }
        m.tryUnload();
    }
}

void HookContext::unload_zygisk() {
    if (flags[CAN_UNLOAD_ZYGISK]) {
        // Do NOT call the destructor
        operator delete(jni_method_map);
        // Directly unmap the whole memory block
        jni_hook::memory_block::release();

        // Strip out all API function pointers
        for (auto &m : modules) {
            m.clearApi();
        }

        new_daemon_thread(reinterpret_cast<thread_entry>(&dlclose), self_handle);
    }
}

// -----------------------------------------------------------------

void HookContext::nativeSpecializeAppProcess_pre() {
    g_ctx = this;
    flags[APP_SPECIALIZE] = true;
    process = env->GetStringUTFChars(args.app->nice_name, nullptr);
    if (flags[APP_FORK_AND_SPECIALIZE]) {
        ZLOGV("pre  forkAndSpecialize [%s]\n", process);
    } else {
        ZLOGV("pre  specialize [%s]\n", process);
    }

    bitset<1024> allowed_fds;
    auto dir = open_dir("/proc/self/fd");
    pre_specialize_start(dir.get(), allowed_fds);

    vector<int> module_fds;
    int fd = remote_get_info(args.app->uid, process, &info_flags, module_fds);
    if ((info_flags & UNMOUNT_MASK) == UNMOUNT_MASK) {
        ZLOGI("[%s] is on the denylist\n", process);
        flags[DO_REVERT_UNMOUNT] = true;
    } else if (fd >= 0) {
        run_modules_pre(module_fds);
    }
    close(fd);

    pre_specialize_end(dir.get(), allowed_fds);
}

void HookContext::nativeSpecializeAppProcess_post() {
    if (flags[APP_FORK_AND_SPECIALIZE]) {
        ZLOGV("post forkAndSpecialize [%s]\n", process);
    } else {
        ZLOGV("post specialize [%s]\n", process);
    }

    env->ReleaseStringUTFChars(args.app->nice_name, process);
    run_modules_post();
    if (info_flags & PROCESS_IS_MAGISK_APP) {
        setenv("ZYGISK_ENABLED", "1", 1);
    }

    // Cleanups
    g_ctx = nullptr;
    close(logd_fd.exchange(-1));
    android_logging();
    if (!flags[APP_FORK_AND_SPECIALIZE]) {
        unload_zygisk();
    }
}

void HookContext::nativeForkSystemServer_pre() {
    fork_pre();
    flags[SERVER_FORK_AND_SPECIALIZE] = true;
    if (pid != 0)
        return;

    ZLOGV("pre  forkSystemServer\n");

    bitset<1024> allowed_fds;
    auto dir = open_dir("/proc/self/fd");
    pre_specialize_start(dir.get(), allowed_fds);

    vector<int> module_fds;
    int fd = remote_get_info(1000, "system_server", &info_flags, module_fds);
    if (fd >= 0) {
        if (module_fds.empty()) {
            write_int(fd, 0);
        } else {
            run_modules_pre(module_fds);

            // Send the bitset of module status back to magiskd from system_server
            dynamic_bitset bits;
            for (const auto &m : modules)
                bits[m.getId()] = true;
            write_int(fd, bits.slots());
            for (int i = 0; i < bits.slots(); ++i) {
                auto l = bits.get_slot(i);
                xwrite(fd, &l, sizeof(l));
            }
        }
        close(fd);
    }

    pre_specialize_end(dir.get(), allowed_fds);
}

void HookContext::nativeForkSystemServer_post() {
    if (pid == 0) {
        ZLOGV("post forkSystemServer\n");
        run_modules_post();
    }
    fork_post();
}

void HookContext::nativeForkAndSpecialize_pre() {
    fork_pre();
    flags[APP_FORK_AND_SPECIALIZE] = true;
    if (pid == 0) {
        nativeSpecializeAppProcess_pre();
    } else if (!setup_fd_ignore()) {
        close(logd_fd.exchange(-1));
    }
}

void HookContext::nativeForkAndSpecialize_post() {
    if (pid == 0) {
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
    unload_zygisk();
}

} // namespace

static bool hook_refresh() {
    if (xhook_refresh(0) == 0) {
        xhook_clear();
        return true;
    } else {
        ZLOGE("xhook failed\n");
        return false;
    }
}

static int hook_register(const char *path, const char *symbol, void *new_func, void **old_func) {
    int ret = xhook_register(path, symbol, new_func, old_func);
    if (ret != 0) {
        ZLOGE("Failed to register hook \"%s\"\n", symbol);
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
    // xhook_enable_debug(1);
    xhook_enable_sigsegv_protection(0);
#endif
    default_new(xhook_list);
    default_new(jni_hook_list);
    default_new(jni_method_map);

    XHOOK_REGISTER(ANDROID_RUNTIME, fork);
    XHOOK_REGISTER(ANDROID_RUNTIME, unshare);
    XHOOK_REGISTER(ANDROID_RUNTIME, jniRegisterNativeMethods);
    XHOOK_REGISTER(ANDROID_RUNTIME, selinux_android_setcontext);
    XHOOK_REGISTER_SYM(ANDROID_RUNTIME, "__android_log_close", android_log_close);
    hook_refresh();

    // Remove unhooked methods
    xhook_list->erase(
            std::remove_if(xhook_list->begin(), xhook_list->end(),
            [](auto &t) { return *std::get<2>(t) == nullptr;}),
            xhook_list->end());

    if (old_jniRegisterNativeMethods == nullptr) {
        ZLOGD("jniRegisterNativeMethods not hooked, using fallback\n");

        // android::AndroidRuntime::setArgv0(const char*, bool)
        XHOOK_REGISTER_SYM(APP_PROCESS, "_ZN7android14AndroidRuntime8setArgv0EPKcb", setArgv0);
        hook_refresh();

        // We still need old_jniRegisterNativeMethods as other code uses it
        // android::AndroidRuntime::registerNativeMethods(_JNIEnv*, const char*, const JNINativeMethod*, int)
        constexpr char sig[] = "_ZN7android14AndroidRuntime21registerNativeMethodsEP7_JNIEnvPKcPK15JNINativeMethodi";
        *(void **) &old_jniRegisterNativeMethods = dlsym(RTLD_DEFAULT, sig);
    }
}

static bool unhook_functions() {
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

    // Unhook JNI methods
    for (const auto &[clz, methods] : *jni_hook_list) {
        if (!methods.empty() && old_jniRegisterNativeMethods(
                g_ctx->env, clz.data(), methods.data(), methods.size()) != 0) {
            ZLOGE("Failed to restore JNI hook of class [%s]\n", clz.data());
            success = false;
        }
    }
    delete jni_hook_list;

    // Unhook xhook
    for (const auto &[path, sym, old_func] : *xhook_list) {
        if (xhook_register(path, sym, *old_func, nullptr) != 0) {
            ZLOGE("Failed to register xhook [%s]\n", sym);
            success = false;
        }
    }
    delete xhook_list;
    if (!hook_refresh()) {
        ZLOGE("Failed to restore xhook\n");
        success = false;
    }

    return success;
}
