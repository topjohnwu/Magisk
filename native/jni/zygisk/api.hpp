// All content of this file is released to the public domain.

// This file is the public API for Zygisk modules.
// DO NOT use this file for developing Zygisk modules as it might contain WIP changes.
// Always use the following header for development as those are finalized APIs:
// https://github.com/topjohnwu/zygisk-module-sample/blob/master/module/jni/zygisk.hpp

#pragma once

#include <jni.h>

#define ZYGISK_API_VERSION 2

/*

Define a class and inherit zygisk::ModuleBase to implement the functionality of your module.
Use the macro REGISTER_ZYGISK_MODULE(className) to register that class to Zygisk.

Please note that modules will only be loaded after zygote has forked the child process.
THIS MEANS ALL OF YOUR CODE RUNS IN THE APP/SYSTEM SERVER PROCESS, NOT THE ZYGOTE DAEMON!

Example code:

static jint (*orig_logger_entry_max)(JNIEnv *env);
static jint my_logger_entry_max(JNIEnv *env) { return orig_logger_entry_max(env); }

static void example_handler(int socket) { ... }

class ExampleModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }
    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        JNINativeMethod methods[] = {
            { "logger_entry_max_payload_native", "()I", (void*) my_logger_entry_max },
        };
        api->hookJniNativeMethods(env, "android/util/Log", methods, 1);
        *(void **) &orig_logger_entry_max = methods[0].fnPtr;
    }
private:
    zygisk::Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(ExampleModule)

REGISTER_ZYGISK_COMPANION(example_handler)

*/

namespace zygisk {

struct Api;
struct AppSpecializeArgs;
struct ServerSpecializeArgs;

class ModuleBase {
public:

    // This function is called when the module is loaded into the target process.
    // A Zygisk API handle will be sent as an argument; call utility functions or interface
    // with Zygisk through this handle.
    virtual void onLoad([[maybe_unused]] Api *api, [[maybe_unused]] JNIEnv *env) {}

    // This function is called before the app process is specialized.
    // At this point, the process just got forked from zygote, but no app specific specialization
    // is applied. This means that the process does not have any sandbox restrictions and
    // still runs with the same privilege of zygote.
    //
    // All the arguments that will be sent and used for app specialization is passed as a single
    // AppSpecializeArgs object. You can read and overwrite these arguments to change how the app
    // process will be specialized.
    //
    // If you need to run some operations as superuser, you can call Api::connectCompanion() to
    // get a socket to do IPC calls with a root companion process.
    // See Api::connectCompanion() for more info.
    virtual void preAppSpecialize([[maybe_unused]] AppSpecializeArgs *args) {}

    // This function is called after the app process is specialized.
    // At this point, the process has all sandbox restrictions enabled for this application.
    // This means that this function runs as the same privilege of the app's own code.
    virtual void postAppSpecialize([[maybe_unused]] const AppSpecializeArgs *args) {}

    // This function is called before the system server process is specialized.
    // See preAppSpecialize(args) for more info.
    virtual void preServerSpecialize([[maybe_unused]] ServerSpecializeArgs *args) {}

    // This function is called after the system server process is specialized.
    // At this point, the process runs with the privilege of system_server.
    virtual void postServerSpecialize([[maybe_unused]] const ServerSpecializeArgs *args) {}
};

struct AppSpecializeArgs {
    // Required arguments. These arguments are guaranteed to exist on all Android versions.
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jint &mount_external;
    jstring &se_info;
    jstring &nice_name;
    jstring &instruction_set;
    jstring &app_data_dir;

    // Optional arguments. Please check whether the pointer is null before de-referencing
    jboolean *const is_child_zygote;
    jboolean *const is_top_app;
    jobjectArray *const pkg_data_info_list;
    jobjectArray *const whitelisted_data_info_list;
    jboolean *const mount_data_dirs;
    jboolean *const mount_storage_dirs;

    AppSpecializeArgs() = delete;
};

struct ServerSpecializeArgs {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jlong &permitted_capabilities;
    jlong &effective_capabilities;

    ServerSpecializeArgs() = delete;
};

namespace internal {
struct api_table;
template <class T> void entry_impl(api_table *, JNIEnv *);
}

// These values are used in Api::setOption(Option)
enum Option : int {
    // Force Magisk's denylist unmount routines to run on this process.
    //
    // Setting this option only makes sense in preAppSpecialize.
    // The actual unmounting happens during app process specialization.
    //
    // Set this option to force all Magisk and modules' files to be unmounted from the
    // mount namespace of the process, regardless of the denylist enforcement status.
    FORCE_DENYLIST_UNMOUNT = 0,

    // When this option is set, your module's library will be dlclose-ed after post[XXX]Specialize.
    // Be aware that after dlclose-ing your module, all of your code will be unmapped from memory.
    // YOU MUST NOT ENABLE THIS OPTION AFTER HOOKING ANY FUNCTIONS IN THE PROCESS.
    DLCLOSE_MODULE_LIBRARY = 1,
};

// Bit masks of the return value of Api::getFlags()
enum StateFlag : uint32_t {
    // The user has granted root access to the current process
    PROCESS_GRANTED_ROOT = (1u << 0),

    // The current process was added on the denylist
    PROCESS_ON_DENYLIST = (1u << 1),
};

// All API functions will stop working after post[XXX]Specialize as Zygisk will be unloaded
// from the specialized process afterwards.
struct Api {

    // Connect to a root companion process and get a Unix domain socket for IPC.
    //
    // This API only works in the pre[XXX]Specialize functions due to SELinux restrictions.
    //
    // The pre[XXX]Specialize functions run with the same privilege of zygote.
    // If you would like to do some operations with superuser permissions, register a handler
    // function that would be called in the root process with REGISTER_ZYGISK_COMPANION(func).
    // Another good use case for a companion process is that if you want to share some resources
    // across multiple processes, hold the resources in the companion process and pass it over.
    //
    // The root companion process is ABI aware; that is, when calling this function from a 32-bit
    // process, you will be connected to a 32-bit companion process, and vice versa for 64-bit.
    //
    // Returns a file descriptor to a socket that is connected to the socket passed to your
    // module's companion request handler. Returns -1 if the connection attempt failed.
    int connectCompanion();

    // Get the file descriptor of the root folder of the current module.
    //
    // This API only works in the pre[XXX]Specialize functions.
    // Accessing the directory returned is only possible in the pre[XXX]Specialize functions
    // or in the root companion process (assuming that you sent the fd over the socket).
    // Both restrictions are due to SELinux and UID.
    //
    // Returns -1 if errors occurred.
    int getModuleDir();

    // Set various options for your module.
    // Please note that this function accepts one single option at a time.
    // Check zygisk::Option for the full list of options available.
    void setOption(Option opt);

    // Get information about the current process.
    // Returns bitwise-or'd zygisk::StateFlag values.
    uint32_t getFlags();

    // Hook JNI native methods for a class
    //
    // Lookup all registered JNI native methods and replace it with your own functions.
    // The original function pointer will be saved in each JNINativeMethod's fnPtr.
    // If no matching class, method name, or signature is found, that specific JNINativeMethod.fnPtr
    // will be set to nullptr.
    void hookJniNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *methods, int numMethods);

    // For ELFs loaded in memory matching `regex`, replace function `symbol` with `newFunc`.
    // If `oldFunc` is not nullptr, the original function pointer will be saved to `oldFunc`.
    void pltHookRegister(const char *regex, const char *symbol, void *newFunc, void **oldFunc);

    // For ELFs loaded in memory matching `regex`, exclude hooks registered for `symbol`.
    // If `symbol` is nullptr, then all symbols will be excluded.
    void pltHookExclude(const char *regex, const char *symbol);

    // Commit all the hooks that was previously registered.
    // Returns false if an error occurred.
    bool pltHookCommit();

private:
    internal::api_table *impl;
    template <class T> friend void internal::entry_impl(internal::api_table *, JNIEnv *);
};

// Register a class as a Zygisk module

#define REGISTER_ZYGISK_MODULE(clazz) \
void zygisk_module_entry(zygisk::internal::api_table *table, JNIEnv *env) { \
    zygisk::internal::entry_impl<clazz>(table, env);                        \
}

// Register a root companion request handler function for your module
//
// The function runs in a superuser daemon process and handles a root companion request from
// your module running in a target process. The function has to accept an integer value,
// which is a socket that is connected to the target process.
// See Api::connectCompanion() for more info.
//
// NOTE: the function can run concurrently on multiple threads.
// Be aware of race conditions if you have a globally shared resource.

#define REGISTER_ZYGISK_COMPANION(func) \
void zygisk_companion_entry(int client) { func(client); }

/************************************************************************************
 * All the code after this point is internal code used to interface with Zygisk
 * and guarantee ABI stability. You do not have to understand what it is doing.
 ************************************************************************************/

namespace internal {

struct module_abi {
    long api_version;
    ModuleBase *_this;

    void (*preAppSpecialize)(ModuleBase *, AppSpecializeArgs *);
    void (*postAppSpecialize)(ModuleBase *, const AppSpecializeArgs *);
    void (*preServerSpecialize)(ModuleBase *, ServerSpecializeArgs *);
    void (*postServerSpecialize)(ModuleBase *, const ServerSpecializeArgs *);

    module_abi(ModuleBase *module) : api_version(ZYGISK_API_VERSION), _this(module) {
        preAppSpecialize = [](auto self, auto args) { self->preAppSpecialize(args); };
        postAppSpecialize = [](auto self, auto args) { self->postAppSpecialize(args); };
        preServerSpecialize = [](auto self, auto args) { self->preServerSpecialize(args); };
        postServerSpecialize = [](auto self, auto args) { self->postServerSpecialize(args); };
    }
};

struct api_table {
    // These first 2 entries are permanent, shall never change
    void *_this;
    bool (*registerModule)(api_table *, module_abi *);

    // Utility functions
    void (*hookJniNativeMethods)(JNIEnv *, const char *, JNINativeMethod *, int);
    void (*pltHookRegister)(const char *, const char *, void *, void **);
    void (*pltHookExclude)(const char *, const char *);
    bool (*pltHookCommit)();

    // Zygisk functions
    int  (*connectCompanion)(void * /* _this */);
    void (*setOption)(void * /* _this */, Option);
    int  (*getModuleDir)(void * /* _this */);
    uint32_t (*getFlags)(void * /* _this */);
};

template <class T>
void entry_impl(api_table *table, JNIEnv *env) {
    ModuleBase *module = new T();
    if (!table->registerModule(table, new module_abi(module)))
        return;
    auto api = new Api();
    api->impl = table;
    module->onLoad(api, env);
}

} // namespace internal

inline int Api::connectCompanion() {
    return impl->connectCompanion ? impl->connectCompanion(impl->_this) : -1;
}
inline int Api::getModuleDir() {
    return impl->getModuleDir ? impl->getModuleDir(impl->_this) : -1;
}
inline void Api::setOption(Option opt) {
    if (impl->setOption) impl->setOption(impl->_this, opt);
}
inline uint32_t Api::getFlags() {
    return impl->getFlags ? impl->getFlags(impl->_this) : 0;
}
inline void Api::hookJniNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *methods, int numMethods) {
    if (impl->hookJniNativeMethods) impl->hookJniNativeMethods(env, className, methods, numMethods);
}
inline void Api::pltHookRegister(const char *regex, const char *symbol, void *newFunc, void **oldFunc) {
    if (impl->pltHookRegister) impl->pltHookRegister(regex, symbol, newFunc, oldFunc);
}
inline void Api::pltHookExclude(const char *regex, const char *symbol) {
    if (impl->pltHookExclude) impl->pltHookExclude(regex, symbol);
}
inline bool Api::pltHookCommit() {
    return impl->pltHookCommit != nullptr && impl->pltHookCommit();
}

} // namespace zygisk

[[gnu::visibility("default")]] [[gnu::used]]
extern "C" void zygisk_module_entry(zygisk::internal::api_table *, JNIEnv *);

[[gnu::visibility("default")]] [[gnu::used]]
extern "C" void zygisk_companion_entry(int);
