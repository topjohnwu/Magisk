module;
#include <sys/mman.h>
#include <android/dlext.h>
#include <dlfcn.h>
#include <lsplt.hpp>
#include <regex.h>
#include <jni.h>
#include "api.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <rust/cxx.h>

export module magisk.zygisk;
export import magisk.core;
export import magisk.deny;

#if defined(__LP64__)
#define ZLOGD(...) LOGD("zygisk64: " __VA_ARGS__)
#define ZLOGE(...) LOGE("zygisk64: " __VA_ARGS__)
#define ZLOGI(...) LOGI("zygisk64: " __VA_ARGS__)
#define ZLOGW(...) LOGW("zygisk64: " __VA_ARGS__)
#else
#define ZLOGD(...) LOGD("zygisk32: " __VA_ARGS__)
#define ZLOGE(...) LOGE("zygisk32: " __VA_ARGS__)
#define ZLOGI(...) LOGI("zygisk32: " __VA_ARGS__)
#define ZLOGW(...) LOGW("zygisk32: " __VA_ARGS__)
#endif

// Extreme verbose logging
// #define ZLOGV(...) ZLOGD(__VA_ARGS__)
#define ZLOGV(...) (void*)0

using namespace std;

int zygisk_request(int req) {
    int fd = connect_daemon(RequestCode::ZYGISK);
    if (fd < 0) return fd;
    write_int(fd, req);
    return fd;
}

int sigmask(int how, int signum) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, signum);
    return sigprocmask(how, &set, nullptr);
}

#define call_app(method)               \
switch (*mod.api_version) {            \
case 1:                                \
case 2: {                              \
    AppSpecializeArgs_v1 a(args);      \
    mod.v1->method(mod.v1->impl, &a);  \
    break;                             \
}                                      \
case 3:                                \
case 4:                                \
case 5:                                \
    mod.v1->method(mod.v1->impl, args);\
    break;                             \
}

export extern "C++" {
struct ZygiskContext;
struct ZygiskModule;

struct AppSpecializeArgs_v1;
using  AppSpecializeArgs_v2 = AppSpecializeArgs_v1;
struct AppSpecializeArgs_v3;
using  AppSpecializeArgs_v4 = AppSpecializeArgs_v3;
struct AppSpecializeArgs_v5;

struct module_abi_v1;
using  module_abi_v2 = module_abi_v1;
using  module_abi_v3 = module_abi_v1;
using  module_abi_v4 = module_abi_v1;
using  module_abi_v5 = module_abi_v1;

struct api_abi_v1;
struct api_abi_v2;
using  api_abi_v3 = api_abi_v2;
struct api_abi_v4;
using  api_abi_v5 = api_abi_v4;

union ApiTable;

struct AppSpecializeArgs_v3 {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jobjectArray &rlimits;
    jint &mount_external;
    jstring &se_info;
    jstring &nice_name;
    jstring &instruction_set;
    jstring &app_data_dir;

    jintArray *fds_to_ignore = nullptr;
    jboolean *is_child_zygote = nullptr;
    jboolean *is_top_app = nullptr;
    jobjectArray *pkg_data_info_list = nullptr;
    jobjectArray *whitelisted_data_info_list = nullptr;
    jboolean *mount_data_dirs = nullptr;
    jboolean *mount_storage_dirs = nullptr;

    AppSpecializeArgs_v3(
            jint &uid, jint &gid, jintArray &gids, jint &runtime_flags,
            jobjectArray &rlimits, jint &mount_external, jstring &se_info, jstring &nice_name,
            jstring &instruction_set, jstring &app_data_dir) :
            uid(uid), gid(gid), gids(gids), runtime_flags(runtime_flags), rlimits(rlimits),
            mount_external(mount_external), se_info(se_info), nice_name(nice_name),
            instruction_set(instruction_set), app_data_dir(app_data_dir) {}
};

struct AppSpecializeArgs_v5 : public AppSpecializeArgs_v3 {
    jboolean *mount_sysprop_overrides = nullptr;

    AppSpecializeArgs_v5(
            jint &uid, jint &gid, jintArray &gids, jint &runtime_flags,
            jobjectArray &rlimits, jint &mount_external, jstring &se_info, jstring &nice_name,
            jstring &instruction_set, jstring &app_data_dir) : AppSpecializeArgs_v3(
                    uid, gid, gids, runtime_flags, rlimits, mount_external,
                    se_info, nice_name, instruction_set, app_data_dir) {}
};

struct AppSpecializeArgs_v1 {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jint &mount_external;
    jstring &se_info;
    jstring &nice_name;
    jstring &instruction_set;
    jstring &app_data_dir;

    jboolean *const is_child_zygote;
    jboolean *const is_top_app;
    jobjectArray *const pkg_data_info_list;
    jobjectArray *const whitelisted_data_info_list;
    jboolean *const mount_data_dirs;
    jboolean *const mount_storage_dirs;

    AppSpecializeArgs_v1(const AppSpecializeArgs_v5 *a) :
            uid(a->uid), gid(a->gid), gids(a->gids), runtime_flags(a->runtime_flags),
            mount_external(a->mount_external), se_info(a->se_info), nice_name(a->nice_name),
            instruction_set(a->instruction_set), app_data_dir(a->app_data_dir),
            is_child_zygote(a->is_child_zygote), is_top_app(a->is_top_app),
            pkg_data_info_list(a->pkg_data_info_list),
            whitelisted_data_info_list(a->whitelisted_data_info_list),
            mount_data_dirs(a->mount_data_dirs), mount_storage_dirs(a->mount_storage_dirs) {}
};

struct ServerSpecializeArgs_v1 {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jlong &permitted_capabilities;
    jlong &effective_capabilities;

    ServerSpecializeArgs_v1(
            jint &uid, jint &gid, jintArray &gids, jint &runtime_flags,
            jlong &permitted_capabilities, jlong &effective_capabilities) :
            uid(uid), gid(gid), gids(gids), runtime_flags(runtime_flags),
            permitted_capabilities(permitted_capabilities),
            effective_capabilities(effective_capabilities) {}
};

struct module_abi_v1 {
    long api_version;
    void *impl;
    void (*preAppSpecialize)(void *, void *);
    void (*postAppSpecialize)(void *, const void *);
    void (*preServerSpecialize)(void *, void *);
    void (*postServerSpecialize)(void *, const void *);
};

// Assert the flag values to be the same as the public API
static_assert(+ZygiskStateFlags::ProcessGrantedRoot == zygisk::StateFlag::PROCESS_GRANTED_ROOT);
static_assert(+ZygiskStateFlags::ProcessOnDenyList == zygisk::StateFlag::PROCESS_ON_DENYLIST);

enum : uint32_t {
    UNMOUNT_MASK = (+ZygiskStateFlags::ProcessOnDenyList | +ZygiskStateFlags::DenyListEnforced),
    PRIVATE_MASK = (+ZygiskStateFlags::DenyListEnforced | +ZygiskStateFlags::ProcessIsMagiskApp)
};

struct api_abi_base {
    ZygiskModule *impl;
    bool (*registerModule)(ApiTable *, long *);
};

struct api_abi_v1 : public api_abi_base {
    /* 0 */ void (*hookJniNativeMethods)(JNIEnv *, const char *, JNINativeMethod *, int);
    /* 1 */ void (*pltHookRegister)(const char *, const char *, void *, void **);
    /* 2 */ void (*pltHookExclude)(const char *, const char *);
    /* 3 */ bool (*pltHookCommit)();
    /* 4 */ int (*connectCompanion)(ZygiskModule *);
    /* 5 */ void (*setOption)(ZygiskModule *, zygisk::Option);
};

struct api_abi_v2 : public api_abi_v1 {
    /* 6 */ int (*getModuleDir)(ZygiskModule *);
    /* 7 */ uint32_t (*getFlags)(ZygiskModule *);
};

struct api_abi_v4 : public api_abi_base {
    /* 0 */ void (*hookJniNativeMethods)(JNIEnv *, const char *, JNINativeMethod *, int);
    /* 1 */ void (*pltHookRegister)(dev_t, ino_t, const char *, void *, void **);
    /* 2 */ bool (*exemptFd)(int);
    /* 3 */ bool (*pltHookCommit)();
    /* 4 */ int (*connectCompanion)(ZygiskModule *);
    /* 5 */ void (*setOption)(ZygiskModule *, zygisk::Option);
    /* 6 */ int (*getModuleDir)(ZygiskModule *);
    /* 7 */ uint32_t (*getFlags)(ZygiskModule *);
};

union ApiTable {
    api_abi_base base;
    api_abi_v1 v1;
    api_abi_v2 v2;
    api_abi_v4 v4;
};

struct ZygiskModule {

    void onLoad(void *env) {
        entry.fn(&api, env);
    }

    void preAppSpecialize(AppSpecializeArgs_v5 *args) const {
        call_app(preAppSpecialize)
    }
    void postAppSpecialize(const AppSpecializeArgs_v5 *args) const {
        call_app(postAppSpecialize)
    }
    void preServerSpecialize(ServerSpecializeArgs_v1 *args) const {
        mod.v1->preServerSpecialize(mod.v1->impl, args);
    }
    void postServerSpecialize(const ServerSpecializeArgs_v1 *args) const {
        mod.v1->postServerSpecialize(mod.v1->impl, args);
    }

    bool valid() const {
        if (mod.api_version == nullptr)
            return false;
        switch (*mod.api_version) {
            case 5:
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
    int connectCompanion() const {
        if (int fd = zygisk_request(+ZygiskRequest::ConnectCompanion); fd >= 0) {
    #ifdef __LP64__
            write_any<bool>(fd, true);
    #else
            write_any<bool>(fd, false);
    #endif
            write_int(fd, id);
            return fd;
        }
        return -1;
    }
    int getModuleDir() const {
        if (owned_fd fd = zygisk_request(+ZygiskRequest::GetModDir); fd >= 0) {
            write_int(fd, id);
            return recv_fd(fd);
        }
        return -1;
    }
    void setOption(zygisk::Option opt);
    static uint32_t getFlags();
    void tryUnload() const {
        if (unload) dlclose(handle);
    }
    void clearApi() { memset(&api, 0, sizeof(api)); }

    ZygiskModule(int id, void *handle, void *entry)
        : id(id), handle(handle), entry{entry}, api{}, mod{nullptr} {
        // Make sure all pointers are null
        memset(&api, 0, sizeof(api));
        api.base.impl = this;
        api.base.registerModule = &ZygiskModule::RegisterModuleImpl;
    }

    static bool RegisterModuleImpl(ApiTable *api, long *module);

private:
    const int id;
    bool unload = false;

    void * const handle;
    union {
        void * const ptr;
        void (* const fn)(void *, void *);
    } entry;

    ApiTable api;

    union {
        long *api_version;
        module_abi_v1 *v1;
    } mod;
};

extern ZygiskContext *g_ctx;
extern int (*old_fork)(void);

enum : uint32_t {
    POST_SPECIALIZE = (1u << 0),
    APP_FORK_AND_SPECIALIZE = (1u << 1),
    APP_SPECIALIZE = (1u << 2),
    SERVER_FORK_AND_SPECIALIZE = (1u << 3),
    DO_REVERT_UNMOUNT = (1u << 4),
    SKIP_CLOSE_LOG_PIPE = (1u << 5),
};

struct ZygiskContext {
    JNIEnv *env;
    union {
        void *ptr;
        AppSpecializeArgs_v5 *app;
        ServerSpecializeArgs_v1 *server;
    } args;

    const char *process;
    std::list<ZygiskModule> modules;

    int pid;
    uint32_t flags;
    uint32_t info_flags;
    std::vector<bool> allowed_fds;
    std::vector<int> exempted_fds;

    struct RegisterInfo {
        regex_t regex;
        std::string symbol;
        void *callback;
        void **backup;
    };

    struct IgnoreInfo {
        regex_t regex;
        std::string symbol;
    };

    pthread_mutex_t hook_info_lock;
    std::vector<RegisterInfo> register_info;
    std::vector<IgnoreInfo> ignore_info;

    ZygiskContext(JNIEnv *env, void *args);
    ~ZygiskContext();

    void run_modules_pre(rust::Vec<int> &fds) {
        for (int i = 0; i < fds.size(); ++i) {
            owned_fd fd = fds[i];
            struct stat s{};
            if (fstat(fd, &s) != 0 || !S_ISREG(s.st_mode)) {
                fds[i] = -1;
                continue;
            }
            android_dlextinfo info {
                .flags = ANDROID_DLEXT_USE_LIBRARY_FD,
                .library_fd = fd,
            };
            if (void *h = android_dlopen_ext("/jit-cache", RTLD_LAZY, &info)) {
                if (void *e = dlsym(h, "zygisk_module_entry")) {
                    modules.emplace_back(i, h, e);
                }
            } else if (flags & SERVER_FORK_AND_SPECIALIZE) {
                ZLOGW("Failed to dlopen zygisk module: %s\n", dlerror());
                fds[i] = -1;
            }
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
            if (flags & APP_SPECIALIZE) {
                m.preAppSpecialize(args.app);
            } else if (flags & SERVER_FORK_AND_SPECIALIZE) {
                m.preServerSpecialize(args.server);
            }
        }
    }
    void run_modules_post() {
        flags |= POST_SPECIALIZE;
        for (const auto &m : modules) {
            if (flags & APP_SPECIALIZE) {
                m.postAppSpecialize(args.app);
            } else if (flags & SERVER_FORK_AND_SPECIALIZE) {
                m.postServerSpecialize(args.server);
            }
            m.tryUnload();
        }
    }
    void fork_pre() {
        // Do our own fork before loading any 3rd party code
        // First block SIGCHLD, unblock after original fork is done
        sigmask(SIG_BLOCK, SIGCHLD);
        pid = old_fork();

        if (!is_child())
            return;

        // Record all open fds
        auto dir = xopen_dir("/proc/self/fd");
        for (dirent *entry; (entry = xreaddir(dir.get()));) {
            int fd = parse_int(entry->d_name);
            if (fd < 0 || fd >= allowed_fds.size()) {
                close(fd);
                continue;
            }
            allowed_fds[fd] = true;
        }
        // The dirfd will be closed once out of scope
        allowed_fds[dirfd(dir.get())] = false;
        // logd_fd should be handled separately
        if (int fd = zygisk_get_logd(); fd >= 0) {
            allowed_fds[fd] = false;
        }
    }
    void fork_post() {
        // Unblock SIGCHLD in case the original method didn't
        sigmask(SIG_UNBLOCK, SIGCHLD);
    }
    void app_specialize_pre() {
        flags |= APP_SPECIALIZE;

        rust::Vec<int> module_fds;
        owned_fd fd = get_module_info(args.app->uid, module_fds);
        if ((info_flags & UNMOUNT_MASK) == UNMOUNT_MASK) {
            ZLOGI("[%s] is on the denylist\n", process);
            flags |= DO_REVERT_UNMOUNT;
        } else if (fd >= 0) {
            run_modules_pre(module_fds);
        }
    }
    void app_specialize_post() {
        run_modules_post();
        if (info_flags & +ZygiskStateFlags::ProcessIsMagiskApp) {
            setenv("ZYGISK_ENABLED", "1", 1);
        }

        // Cleanups
        env->ReleaseStringUTFChars(args.app->nice_name, process);
    }
    void server_specialize_pre() {
        rust::Vec<int> module_fds;
        if (owned_fd fd = get_module_info(1000, module_fds); fd >= 0) {
            if (module_fds.empty()) {
                write_int(fd, 0);
            } else {
                run_modules_pre(module_fds);

                // Find all failed module ids and send it back to magiskd
                vector<int> failed_ids;
                for (int i = 0; i < module_fds.size(); ++i) {
                    if (module_fds[i] < 0) {
                        failed_ids.push_back(i);
                    }
                }
                write_vector(fd, failed_ids);
            }
        }
    }
    void server_specialize_post() {
        run_modules_post();
    }
    void nativeForkAndSpecialize_pre() {
        process = env->GetStringUTFChars(args.app->nice_name, nullptr);
        ZLOGV("pre  forkAndSpecialize [%s]\n", process);
        flags |= APP_FORK_AND_SPECIALIZE;

        fork_pre();
        if (is_child()) {
            app_specialize_pre();
        }
        sanitize_fds();
    }
    void nativeForkAndSpecialize_post() {
        if (is_child()) {
            ZLOGV("post forkAndSpecialize [%s]\n", process);
            app_specialize_post();
        }
        fork_post();
    }
    void nativeSpecializeAppProcess_pre() {
        process = env->GetStringUTFChars(args.app->nice_name, nullptr);
        ZLOGV("pre  specialize [%s]\n", process);
        // App specialize does not check FD
        flags |= SKIP_CLOSE_LOG_PIPE;
        app_specialize_pre();
    }
    void nativeSpecializeAppProcess_post() {
        ZLOGV("post specialize [%s]\n", process);
        app_specialize_post();
    }
    void nativeForkSystemServer_pre() {
        ZLOGV("pre  forkSystemServer\n");
        flags |= SERVER_FORK_AND_SPECIALIZE;
        process = "system_server";

        fork_pre();
        if (is_child()) {
            server_specialize_pre();
        }
        sanitize_fds();
    }
    void nativeForkSystemServer_post() {
        if (is_child()) {
            ZLOGV("post forkSystemServer\n");
            server_specialize_post();
        }
        fork_post();
    }

    int get_module_info(int uid, rust::Vec<int> &fds) {
        if (int fd = zygisk_request(+ZygiskRequest::GetInfo); fd >= 0) {
            write_int(fd, uid);
            write_string(fd, process);
    #ifdef __LP64__
            write_any<bool>(fd, true);
    #else
            write_any<bool>(fd, false);
    #endif
            xxread(fd, &info_flags, sizeof(info_flags));
            if (zygisk_should_load_module(info_flags)) {
                fds = recv_fds(fd);
            }
            return fd;
        }
        return -1;
    }
    void sanitize_fds() {
        zygisk_close_logd();

        if (!is_child()) {
            return;
        }

        if (can_exempt_fd() && !exempted_fds.empty()) {
            auto update_fd_array = [&](int old_len) -> jintArray {
                jintArray array = env->NewIntArray(static_cast<int>(old_len + exempted_fds.size()));
                if (array == nullptr)
                    return nullptr;

                env->SetIntArrayRegion(
                        array, old_len, static_cast<int>(exempted_fds.size()), exempted_fds.data());
                for (int fd : exempted_fds) {
                    if (fd >= 0 && fd < allowed_fds.size()) {
                        allowed_fds[fd] = true;
                    }
                }
                *args.app->fds_to_ignore = array;
                return array;
            };

            if (jintArray fdsToIgnore = *args.app->fds_to_ignore) {
                int *arr = env->GetIntArrayElements(fdsToIgnore, nullptr);
                int len = env->GetArrayLength(fdsToIgnore);
                for (int i = 0; i < len; ++i) {
                    int fd = arr[i];
                    if (fd >= 0 && fd < allowed_fds.size()) {
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

        // Close all forbidden fds to prevent crashing
        auto dir = xopen_dir("/proc/self/fd");
        int dfd = dirfd(dir.get());
        for (dirent *entry; (entry = xreaddir(dir.get()));) {
            int fd = parse_int(entry->d_name);
            if ((fd < 0 || fd >= allowed_fds.size() || !allowed_fds[fd]) && fd != dfd) {
                close(fd);
            }
        }
    }
    bool exempt_fd(int fd) {
        if ((flags & POST_SPECIALIZE) || (flags & SKIP_CLOSE_LOG_PIPE))
            return true;
        if (!can_exempt_fd())
            return false;
        exempted_fds.push_back(fd);
        return true;
    }
    bool can_exempt_fd() const {
        return (flags & APP_FORK_AND_SPECIALIZE) && args.app->fds_to_ignore;
    }
    bool is_child() const { return pid <= 0; }

    // Compatibility shim
    void plt_hook_register(const char *regex, const char *symbol, void *fn, void **backup) {
        if (regex == nullptr || symbol == nullptr || fn == nullptr)
            return;
        regex_t re;
        if (regcomp(&re, regex, REG_NOSUB) != 0)
            return;
        mutex_guard lock(hook_info_lock);
        register_info.emplace_back(RegisterInfo{re, symbol, fn, backup});
    }
    void plt_hook_exclude(const char *regex, const char *symbol) {
        if (!regex) return;
        regex_t re;
        if (regcomp(&re, regex, REG_NOSUB) != 0)
            return;
        mutex_guard lock(hook_info_lock);
        ignore_info.emplace_back(IgnoreInfo{re, symbol ?: ""});
    }
    void plt_hook_process_regex() {
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
                    lsplt::RegisterHook(map.dev, map.inode, reg.symbol, reg.callback, reg.backup);
                }
            }
        }
    }

    bool plt_hook_commit() {
        {
            mutex_guard lock(hook_info_lock);
            plt_hook_process_regex();
            for (auto& reg: register_info) {
                regfree(&reg.regex);
            }
            for (auto& ign: ignore_info) {
                regfree(&ign.regex);
            }
            register_info.clear();
            ignore_info.clear();
        }
        return lsplt::CommitHook();
    }
};

void hook_entry();
int zygisk_main(int argc, char *argv[]);
void hookJniNativeMethods(JNIEnv *env, const char *clz, JNINativeMethod *methods, int numMethods);

// The reference of the following structs
// https://cs.android.com/android/platform/superproject/main/+/main:art/libnativebridge/include/nativebridge/native_bridge.h

struct NativeBridgeRuntimeCallbacks {
    const char* (*getMethodShorty)(JNIEnv* env, jmethodID mid);
    uint32_t (*getNativeMethodCount)(JNIEnv* env, jclass clazz);
    uint32_t (*getNativeMethods)(JNIEnv* env, jclass clazz, JNINativeMethod* methods,
                                 uint32_t method_count);
};

struct NativeBridgeCallbacks {
    uint32_t version;
    void *padding[5];
    bool (*isCompatibleWith)(uint32_t);
};

inline constexpr char ZYGISKLDR[] = "libzygisk.so";
inline constexpr char NBPROP[] = "ro.dalvik.vm.native.bridge";
}

using namespace std;

extern "C++" bool ZygiskModule::RegisterModuleImpl(ApiTable *api, long *module) {
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
        api->v4.pltHookCommit = lsplt::CommitHook;
        api->v4.pltHookRegister = [](dev_t dev, ino_t inode, const char *symbol, void *fn, void **backup) {
            if (dev == 0 || inode == 0 || symbol == nullptr || fn == nullptr)
                return;
            lsplt::RegisterHook(dev, inode, symbol, fn, backup);
        };
        api->v4.exemptFd = [](int fd) { return g_ctx && g_ctx->exempt_fd(fd); };
    }

    return true;
}

extern "C++" void ZygiskModule::setOption(zygisk::Option opt) {
    if (g_ctx == nullptr)
        return;
    switch (opt) {
        case zygisk::FORCE_DENYLIST_UNMOUNT:
            g_ctx->flags |= DO_REVERT_UNMOUNT;
            break;
        case zygisk::DLCLOSE_MODULE_LIBRARY:
            unload = true;
            break;
    }
}

extern "C++" uint32_t ZygiskModule::getFlags() {
    return g_ctx ? (g_ctx->info_flags & ~PRIVATE_MASK) : 0;
}

// -----------------------------------------------------------------

// -----------------------------------------------------------------

// -----------------------------------------------------------------

// -----------------------------------------------------------------
