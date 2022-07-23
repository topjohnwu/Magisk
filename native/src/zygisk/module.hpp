#pragma once

#include "api.hpp"

namespace {

struct HookContext;
struct ZygiskModule;

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

    AppSpecializeArgs_v1(const AppSpecializeArgs_v3 *v3) :
            uid(v3->uid), gid(v3->gid), gids(v3->gids), runtime_flags(v3->runtime_flags),
            mount_external(v3->mount_external), se_info(v3->se_info), nice_name(v3->nice_name),
            instruction_set(v3->instruction_set), app_data_dir(v3->app_data_dir),
            is_child_zygote(v3->is_child_zygote), is_top_app(v3->is_top_app),
            pkg_data_info_list(v3->pkg_data_info_list),
            whitelisted_data_info_list(v3->whitelisted_data_info_list),
            mount_data_dirs(v3->mount_data_dirs), mount_storage_dirs(v3->mount_storage_dirs) {}
};

struct module_abi_raw {
    long api_version;
    void *_this;
    void (*preAppSpecialize)(void *, void *);
    void (*postAppSpecialize)(void *, const void *);
    void (*preServerSpecialize)(void *, void *);
    void (*postServerSpecialize)(void *, const void *);
};

using module_abi_v1 = module_abi_raw;

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

enum : uint32_t {
    PROCESS_GRANTED_ROOT = zygisk::StateFlag::PROCESS_GRANTED_ROOT,
    PROCESS_ON_DENYLIST = zygisk::StateFlag::PROCESS_ON_DENYLIST,

    DENYLIST_ENFORCING = (1u << 30),
    PROCESS_IS_MAGISK_APP = (1u << 31),

    UNMOUNT_MASK = (PROCESS_ON_DENYLIST | DENYLIST_ENFORCING),
    PRIVATE_MASK = (0x3u << 30)
};

struct ApiTable {
    // These first 2 entries are permanent
    ZygiskModule *module;
    bool (*registerModule)(ApiTable *, long *);

    struct {
        void (*hookJniNativeMethods)(JNIEnv *, const char *, JNINativeMethod *, int);
        void (*pltHookRegister)(const char *, const char *, void *, void **);
        void (*pltHookExclude)(const char *, const char *);
        bool (*pltHookCommit)();

        int (*connectCompanion)(ZygiskModule *);
        void (*setOption)(ZygiskModule *, zygisk::Option);
    } v1{};
    struct {
        int (*getModuleDir)(ZygiskModule *);
        uint32_t (*getFlags)(ZygiskModule *);
    } v2{};

    ApiTable(ZygiskModule *m);
};

#define call_app(method)            \
switch (*ver) {                     \
case 1:                             \
case 2: {                           \
    AppSpecializeArgs_v1 a(args);   \
    v1->method(v1->_this, &a);      \
    break;                          \
}                                   \
case 3:                             \
    v1->method(v1->_this, args);    \
    break;                          \
}

struct ZygiskModule {

    void preAppSpecialize(AppSpecializeArgs_v3 *args) const {
        call_app(preAppSpecialize)
    }
    void postAppSpecialize(const AppSpecializeArgs_v3 *args) const {
        call_app(postAppSpecialize)
    }
    void preServerSpecialize(ServerSpecializeArgs_v1 *args) const {
        v1->preServerSpecialize(v1->_this, args);
    }
    void postServerSpecialize(const ServerSpecializeArgs_v1 *args) const {
        v1->postServerSpecialize(v1->_this, args);
    }

    int connectCompanion() const;
    int getModuleDir() const;
    void setOption(zygisk::Option opt);
    static uint32_t getFlags();
    void doUnload() const { if (unload) dlclose(handle); }
    int getId() const { return id; }

    ZygiskModule(int id, void *handle, void *entry);

    static bool RegisterModule(ApiTable *table, long *module);

    union {
        void (* const entry)(void *, void *);
        void * const raw_entry;
    };
    ApiTable api;

private:
    const int id;
    bool unload = false;
    void * const handle;
    union {
        long *ver = nullptr;
        module_abi_v1 *v1;
    };
};

} // namespace
