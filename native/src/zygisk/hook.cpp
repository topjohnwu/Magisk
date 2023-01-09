#include <android/dlext.h>
#include <sys/mount.h>
#include <dlfcn.h>
#include <regex.h>
#include <bitset>
#include <list>

#include <lsplt.hpp>

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
#define ZLOGV(...) (void*)0

static bool unhook_functions();

namespace {

enum {
    POST_SPECIALIZE,
    APP_FORK_AND_SPECIALIZE,
    APP_SPECIALIZE,
    SERVER_FORK_AND_SPECIALIZE,
    DO_REVERT_UNMOUNT,
    CAN_UNLOAD_ZYGISK,
    SKIP_FD_SANITIZATION,

    FLAG_MAX
};

#define DCL_PRE_POST(name) \
void name##_pre();         \
void name##_post();

#define MAX_FD_SIZE 1024

struct HookContext {
    JNIEnv *env;
    union {
        void *ptr;
        AppSpecializeArgs_v3 *app;
        ServerSpecializeArgs_v1 *server;
    } args;

    const char *process;
    list<ZygiskModule> modules;

    int pid;
    bitset<FLAG_MAX> flags;
    uint32_t info_flags;
    bitset<MAX_FD_SIZE> allowed_fds;
    vector<int> exempted_fds;

    struct RegisterInfo {
        regex_t regex;
        string symbol;
        void *callback;
        void **backup;
    };

    struct IgnoreInfo {
        regex_t regex;
        string symbol;
    };

    pthread_mutex_t hook_info_lock;
    vector<RegisterInfo> register_info;
    vector<IgnoreInfo> ignore_info;

    HookContext() :
    env(nullptr), args{nullptr}, process(nullptr), pid(-1), info_flags(0),
    hook_info_lock(PTHREAD_MUTEX_INITIALIZER) {}

    void run_modules_pre(const vector<int> &fds);
    void run_modules_post();
    DCL_PRE_POST(fork)
    DCL_PRE_POST(app_specialize)
    DCL_PRE_POST(nativeForkAndSpecialize)
    DCL_PRE_POST(nativeSpecializeAppProcess)
    DCL_PRE_POST(nativeForkSystemServer)

    void unload_zygisk();
    void sanitize_fds();
    bool exempt_fd(int fd);

    // Compatibility shim
    void plt_hook_register(const char *regex, const char *symbol, void *fn, void **backup);
    void plt_hook_exclude(const char *regex, const char *symbol);
    void plt_hook_process_regex();

    bool plt_hook_commit();
};

#undef DCL_PRE_POST

// Global variables
vector<tuple<ino_t, const char *, void **>> *plt_hook_list;
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
        (g_ctx->info_flags & PROCESS_IS_SYS_UI) == 0) {
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

// Close logd_fd if necessary to prevent crashing
// For more info, check comments in zygisk_log_write
DCL_HOOK_FUNC(void, android_log_close) {
    if (g_ctx == nullptr) {
        // Happens during un-managed fork like nativeForkApp, nativeForkUsap
        close(logd_fd.exchange(-1));
    } else if (!g_ctx->flags[SKIP_FD_SANITIZATION]) {
        close(logd_fd.exchange(-1));
        if (g_ctx->pid <= 0) {
            // Switch to plain old android logging because we cannot talk
            // to magiskd to fetch our log pipe afterwards anyways.
            android_logging();
        }
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

bool ZygiskModule::RegisterModuleImpl(ApiTable *api, long *module) {
    if (api == nullptr || module == nullptr)
        return false;

    long api_version = *module;
    // Unsupported version
    if (api_version > ZYGISK_API_VERSION)
        return false;

    // Set the actual module_abi*
    api->base.impl->mod = { module };

    // Fill in API accordingly with module API version
    if (api_version >= 1) {
        api->v1.hookJniNativeMethods = hookJniNativeMethods;
        api->v1.pltHookRegister = [](auto a, auto b, auto c, auto d) {
            if (g_ctx) g_ctx->plt_hook_register(a, b, c, d);
        };
        api->v1.pltHookExclude = [](auto a, auto b) {
            if (g_ctx) g_ctx->plt_hook_exclude(a, b);
        };
        api->v1.pltHookCommit = []() { return g_ctx && g_ctx->plt_hook_commit(); };
        api->v1.connectCompanion = [](ZygiskModule *m) { return m->connectCompanion(); };
        api->v1.setOption = [](ZygiskModule *m, auto opt) { m->setOption(opt); };
    }
    if (api_version >= 2) {
        api->v2.getModuleDir = [](ZygiskModule *m) { return m->getModuleDir(); };
        api->v2.getFlags = [](auto) { return ZygiskModule::getFlags(); };
    }
    if (api_version >= 4) {
        api->v4.pltHookRegister = [](ino_t inode, const char *symbol, void *fn, void **backup) {
            if (inode == 0 || symbol == nullptr || fn == nullptr)
                return;
            lsplt::RegisterHook(inode, symbol, fn, backup);
        };
        api->v4.exemptFd = [](int fd) { return g_ctx && g_ctx->exempt_fd(fd); };
    }

    return true;
}

void HookContext::plt_hook_register(const char *regex, const char *symbol, void *fn, void **backup) {
    if (regex == nullptr || symbol == nullptr || fn == nullptr)
        return;
    regex_t re;
    if (regcomp(&re, regex, REG_NOSUB) != 0)
        return;
    mutex_guard lock(hook_info_lock);
    register_info.emplace_back(RegisterInfo{re, symbol, fn, backup});
}

void HookContext::plt_hook_exclude(const char *regex, const char *symbol) {
    if (!regex) return;
    regex_t re;
    if (regcomp(&re, regex, REG_NOSUB) != 0)
        return;
    mutex_guard lock(hook_info_lock);
    ignore_info.emplace_back(IgnoreInfo{re, symbol ?: ""});
}

void HookContext::plt_hook_process_regex() {
    if (register_info.empty())
        return;
    for (auto &map : lsplt::MapInfo::Scan()) {
        if (map.offset != 0 || !map.is_private || !(map.perms & PROT_READ)) continue;
        for (auto &reg: register_info) {
            if (regexec(&reg.regex, map.path.data(), 0, nullptr, 0) != 0)
                continue;
            bool ignored = false;
            for (auto &ign: ignore_info) {
                if (regexec(&ign.regex, map.path.data(), 0, nullptr, 0) != 0)
                    continue;
                if (ign.symbol.empty() || ign.symbol == reg.symbol) {
                    ignored = true;
                    break;
                }
            }
            if (!ignored) {
                lsplt::RegisterHook(map.inode, reg.symbol, reg.callback, reg.backup);
            }
        }
    }
}

bool HookContext::plt_hook_commit() {
    {
        mutex_guard lock(hook_info_lock);
        plt_hook_process_regex();
        register_info.clear();
        ignore_info.clear();
    }
    return lsplt::CommitHook();
}


bool ZygiskModule::valid() const {
    if (mod.api_version == nullptr)
        return false;
    switch (*mod.api_version) {
    case 4:
    case 3:
    case 2:
    case 1:
        return mod.v1->impl && mod.v1->preAppSpecialize && mod.v1->postAppSpecialize &&
            mod.v1->preServerSpecialize && mod.v1->postServerSpecialize;
    default:
        return false;
    }
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

int sigmask(int how, int signum) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, signum);
    return sigprocmask(how, &set, nullptr);
}

void HookContext::fork_pre() {
    g_ctx = this;
    // Do our own fork before loading any 3rd party code
    // First block SIGCHLD, unblock after original fork is done
    sigmask(SIG_BLOCK, SIGCHLD);
    pid = old_fork();
    if (pid != 0 || flags[SKIP_FD_SANITIZATION])
        return;

    // Record all open fds
    auto dir = xopen_dir("/proc/self/fd");
    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        int fd = parse_int(entry->d_name);
        if (fd < 0 || fd >= MAX_FD_SIZE) {
            close(fd);
            continue;
        }
        allowed_fds[fd] = true;
    }
    // The dirfd should not be allowed
    allowed_fds[dirfd(dir.get())] = false;
}

void HookContext::sanitize_fds() {
    if (flags[SKIP_FD_SANITIZATION])
        return;

    if (flags[APP_FORK_AND_SPECIALIZE]) {
        auto update_fd_array = [&](int off) -> jintArray {
            if (exempted_fds.empty())
                return nullptr;

            jintArray array = env->NewIntArray(off + exempted_fds.size());
            if (array == nullptr)
                return nullptr;

            env->SetIntArrayRegion(array, off, exempted_fds.size(), exempted_fds.data());
            for (int fd : exempted_fds) {
                if (fd >= 0 && fd < MAX_FD_SIZE) {
                    allowed_fds[fd] = true;
                }
            }
            *args.app->fds_to_ignore = array;
            flags[SKIP_FD_SANITIZATION] = true;
            return array;
        };

        if (jintArray fdsToIgnore = *args.app->fds_to_ignore) {
            int *arr = env->GetIntArrayElements(fdsToIgnore, nullptr);
            int len = env->GetArrayLength(fdsToIgnore);
            for (int i = 0; i < len; ++i) {
                int fd = arr[i];
                if (fd >= 0 && fd < MAX_FD_SIZE) {
                    allowed_fds[fd] = true;
                }
            }
            if (jintArray newFdList = update_fd_array(len)) {
                env->SetIntArrayRegion(newFdList, 0, len, arr);
            }
            env->ReleaseIntArrayElements(fdsToIgnore, arr, JNI_ABORT);
        } else {
            update_fd_array(0);
        }
    }

    if (pid != 0)
        return;

    // Close all forbidden fds to prevent crashing
    auto dir = xopen_dir("/proc/self/fd");
    int dfd = dirfd(dir.get());
    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        int fd = parse_int(entry->d_name);
        if ((fd < 0 || fd >= MAX_FD_SIZE || !allowed_fds[fd]) && fd != dfd) {
            close(fd);
        }
    }
}

void HookContext::fork_post() {
    // Unblock SIGCHLD in case the original method didn't
    sigmask(SIG_UNBLOCK, SIGCHLD);
    g_ctx = nullptr;
    unload_zygisk();
}

void HookContext::run_modules_pre(const vector<int> &fds) {
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

    for (auto it = modules.begin(); it != modules.end();) {
        it->onLoad(env);
        if (it->valid()) {
            ++it;
        } else {
            it = modules.erase(it);
        }
    }

    for (auto &m : modules) {
        if (flags[APP_SPECIALIZE]) {
            m.preAppSpecialize(args.app);
        } else if (flags[SERVER_FORK_AND_SPECIALIZE]) {
            m.preServerSpecialize(args.server);
        }
    }
}

void HookContext::run_modules_post() {
    flags[POST_SPECIALIZE] = true;
    for (const auto &m : modules) {
        if (flags[APP_SPECIALIZE]) {
            m.postAppSpecialize(args.app);
        } else if (flags[SERVER_FORK_AND_SPECIALIZE]) {
            m.postServerSpecialize(args.server);
        }
        m.tryUnload();
    }
}

void HookContext::app_specialize_pre() {
    flags[APP_SPECIALIZE] = true;

    vector<int> module_fds;
    int fd = remote_get_info(args.app->uid, process, &info_flags, module_fds);
    if ((info_flags & UNMOUNT_MASK) == UNMOUNT_MASK) {
        ZLOGI("[%s] is on the denylist\n", process);
        flags[DO_REVERT_UNMOUNT] = true;
    } else if (fd >= 0) {
        run_modules_pre(module_fds);
    }
    close(fd);
}


void HookContext::app_specialize_post() {
    run_modules_post();
    if (info_flags & PROCESS_IS_MAGISK_APP) {
        setenv("ZYGISK_ENABLED", "1", 1);
    }

    // Cleanups
    env->ReleaseStringUTFChars(args.app->nice_name, process);
    g_ctx = nullptr;
    close(logd_fd.exchange(-1));
    android_logging();
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

bool HookContext::exempt_fd(int fd) {
    if (flags[POST_SPECIALIZE] || flags[SKIP_FD_SANITIZATION])
        return true;
    if (!flags[APP_FORK_AND_SPECIALIZE])
        return false;
    exempted_fds.push_back(fd);
    return true;
}

// -----------------------------------------------------------------

void HookContext::nativeSpecializeAppProcess_pre() {
    process = env->GetStringUTFChars(args.app->nice_name, nullptr);
    ZLOGV("pre  specialize [%s]\n", process);
    g_ctx = this;
    // App specialize does not check FD
    flags[SKIP_FD_SANITIZATION] = true;
    app_specialize_pre();
}

void HookContext::nativeSpecializeAppProcess_post() {
    ZLOGV("post specialize [%s]\n", process);
    app_specialize_post();
    unload_zygisk();
}

void HookContext::nativeForkSystemServer_pre() {
    ZLOGV("pre  forkSystemServer\n");
    flags[SERVER_FORK_AND_SPECIALIZE] = true;

    fork_pre();
    if (pid != 0)
        return;

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

    sanitize_fds();
}

void HookContext::nativeForkSystemServer_post() {
    if (pid == 0) {
        ZLOGV("post forkSystemServer\n");
        run_modules_post();
    }
    fork_post();
}

void HookContext::nativeForkAndSpecialize_pre() {
    process = env->GetStringUTFChars(args.app->nice_name, nullptr);
    ZLOGV("pre  forkAndSpecialize [%s]\n", process);

    flags[APP_FORK_AND_SPECIALIZE] = true;
    if (args.app->fds_to_ignore == nullptr) {
        // The field fds_to_ignore don't exist before Android 8.0, which FDs are not checked
        flags[SKIP_FD_SANITIZATION] = true;
    } else if (logd_fd >= 0) {
        exempted_fds.push_back(logd_fd);
    }

    fork_pre();
    if (pid == 0) {
        app_specialize_pre();
    }
    sanitize_fds();
}

void HookContext::nativeForkAndSpecialize_post() {
    if (pid == 0) {
        ZLOGV("post forkAndSpecialize [%s]\n", process);
        app_specialize_post();
    }
    fork_post();
}

} // namespace

static bool hook_commit() {
    if (lsplt::CommitHook()) {
        return true;
    } else {
        ZLOGE("plt_hook failed\n");
        return false;
    }
}

static void hook_register(ino_t inode, const char *symbol, void *new_func, void **old_func) {
    if (!lsplt::RegisterHook(inode, symbol, new_func, old_func)) {
        ZLOGE("Failed to register plt_hook \"%s\"\n", symbol);
        return;
    }
    plt_hook_list->emplace_back(inode, symbol, old_func);
}

#define PLT_HOOK_REGISTER_SYM(PATH_REGEX, SYM, NAME) \
    hook_register(PATH_REGEX, SYM, (void*) new_##NAME, (void **) &old_##NAME)

#define PLT_HOOK_REGISTER(PATH_REGEX, NAME) \
    PLT_HOOK_REGISTER_SYM(PATH_REGEX, #NAME, NAME)

void hook_functions() {
    default_new(plt_hook_list);
    default_new(jni_hook_list);
    default_new(jni_method_map);

    ino_t android_runtime_inode = 0;
    for (auto &map : lsplt::MapInfo::Scan()) {
        if (map.path.ends_with("libandroid_runtime.so")) {
            android_runtime_inode = map.inode;
            break;
        }
    }

    PLT_HOOK_REGISTER(android_runtime_inode, fork);
    PLT_HOOK_REGISTER(android_runtime_inode, unshare);
    PLT_HOOK_REGISTER(android_runtime_inode, jniRegisterNativeMethods);
    PLT_HOOK_REGISTER(android_runtime_inode, selinux_android_setcontext);
    PLT_HOOK_REGISTER_SYM(android_runtime_inode, "__android_log_close", android_log_close);
    hook_commit();

    // Remove unhooked methods
    plt_hook_list->erase(
            std::remove_if(plt_hook_list->begin(), plt_hook_list->end(),
            [](auto &t) { return *std::get<2>(t) == nullptr;}),
            plt_hook_list->end());

    if (old_jniRegisterNativeMethods == nullptr) {
        ZLOGD("jniRegisterNativeMethods not hooked, using fallback\n");
        struct stat self_stat{};
        stat("/proc/self/exe", &self_stat);
        // android::AndroidRuntime::setArgv0(const char*, bool)
        PLT_HOOK_REGISTER_SYM(self_stat.st_ino, "_ZN7android14AndroidRuntime8setArgv0EPKcb", setArgv0);
        hook_commit();

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

    // Unhook plt_hook
    for (const auto &[inode, sym, old_func] : *plt_hook_list) {
        if (!lsplt::RegisterHook(inode, sym, *old_func, nullptr)) {
            ZLOGE("Failed to register plt_hook [%s]\n", sym);
            success = false;
        }
    }
    delete plt_hook_list;
    if (!hook_commit()) {
        ZLOGE("Failed to restore plt_hook\n");
        success = false;
    }

    return success;
}
