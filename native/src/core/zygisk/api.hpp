/* Copyright 2022-2023 John "topjohnwu" Wu
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

// This is the public API for Zygisk modules.
// DO NOT MODIFY ANY CODE IN THIS HEADER.

// WARNING: this file may contain changes that are not finalized.
// Always use the following published header for development:
// https://github.com/topjohnwu/zygisk-module-sample/blob/master/module/jni/zygisk.hpp

#pragma once

#include <jni.h>

#define ZYGISK_API_VERSION 5

/*

***************
* Introduction
***************

On Android, all app processes are forked from a special daemon called "Zygote".
For each new app process, zygote will fork a new process and perform "specialization".
This specialization operation enforces the Android security sandbox on the newly forked
process to make sure that 3rd party application code is only loaded after it is being
restricted within a sandbox.

On Android, there is also this special process called "system_server". This single
process hosts a significant portion of system services, which controls how the
Android operating system and apps interact with each other.

The Zygisk framework provides a way to allow developers to build modules and run custom
code before and after system_server and any app processes' specialization.
This enable developers to inject code and alter the behavior of system_server and app processes.

Please note that modules will only be loaded after zygote has forked the child process.
THIS MEANS ALL OF YOUR CODE RUNS IN THE APP/SYSTEM_SERVER PROCESS, NOT THE ZYGOTE DAEMON!

*********************
* Development Guide
*********************

Define a class and inherit zygisk::ModuleBase to implement the functionality of your module.
Use the macro REGISTER_ZYGISK_MODULE(className) to register that class to Zygisk.

Example code:

static jint (*orig_logger_entry_max)(JNIEnv *env);
static jint my_logger_entry_max(JNIEnv *env) { return orig_logger_entry_max(env); }

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

-----------------------------------------------------------------------------------------

Since your module class's code runs with either Zygote's privilege in pre[XXX]Specialize,
or runs in the sandbox of the target process in post[XXX]Specialize, the code in your class
never runs in a true superuser environment.

If your module require access to superuser permissions, you can create and register
a root companion handler function. This function runs in a separate root companion
daemon process, and an Unix domain socket is provided to allow you to perform IPC between
your target process and the root companion process.

Example code:

static void example_handler(int socket) { ... }

REGISTER_ZYGISK_COMPANION(example_handler)

*/

namespace zygisk {

struct Api;
struct AppSpecializeArgs;
struct ServerSpecializeArgs;

class ModuleBase {
public:

    // This method is called as soon as the module is loaded into the target process.
    // A Zygisk API handle will be passed as an argument.
    virtual void onLoad([[maybe_unused]] Api *api, [[maybe_unused]] JNIEnv *env) {}

    // This method is called before the app process is specialized.
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

    // This method is called after the app process is specialized.
    // At this point, the process has all sandbox restrictions enabled for this application.
    // This means that this method runs with the same privilege of the app's own code.
    virtual void postAppSpecialize([[maybe_unused]] const AppSpecializeArgs *args) {}

    // This method is called before the system server process is specialized.
    // See preAppSpecialize(args) for more info.
    virtual void preServerSpecialize([[maybe_unused]] ServerSpecializeArgs *args) {}

    // This method is called after the system server process is specialized.
    // At this point, the process runs with the privilege of system_server.
    virtual void postServerSpecialize([[maybe_unused]] const ServerSpecializeArgs *args) {}
};

struct AppSpecializeArgs {
    // Required arguments. These arguments are guaranteed to exist on all Android versions.
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

    // Optional arguments. Please check whether the pointer is null before de-referencing
    jintArray *const fds_to_ignore;
    jboolean *const is_child_zygote;
    jboolean *const is_top_app;
    jobjectArray *const pkg_data_info_list;
    jobjectArray *const whitelisted_data_info_list;
    jboolean *const mount_data_dirs;
    jboolean *const mount_storage_dirs;
    jboolean *const mount_sysprop_overrides;

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

// All API methods will stop working after post[XXX]Specialize as Zygisk will be unloaded
// from the specialized process afterwards.
struct Api {

    // Connect to a root companion process and get a Unix domain socket for IPC.
    //
    // This API only works in the pre[XXX]Specialize methods due to SELinux restrictions.
    //
    // The pre[XXX]Specialize methods run with the same privilege of zygote.
    // If you would like to do some operations with superuser permissions, register a handler
    // function that would be called in the root process with REGISTER_ZYGISK_COMPANION(func).
    // Another good use case for a companion process is that if you want to share some resources
    // across multiple processes, hold the resources in the companion process and pass it over.
    //
    // The root companion process is ABI aware; that is, when calling this method from a 32-bit
    // process, you will be connected to a 32-bit companion process, and vice versa for 64-bit.
    //
    // Returns a file descriptor to a socket that is connected to the socket passed to your
    // module's companion request handler. Returns -1 if the connection attempt failed.
    int connectCompanion();

    // Get the file descriptor of the root folder of the current module.
    //
    // This API only works in the pre[XXX]Specialize methods.
    // Accessing the directory returned is only possible in the pre[XXX]Specialize methods
    // or in the root companion process (assuming that you sent the fd over the socket).
    // Both restrictions are due to SELinux and UID.
    //
    // Returns -1 if errors occurred.
    int getModuleDir();

    // Set various options for your module.
    // Please note that this method accepts one single option at a time.
    // Check zygisk::Option for the full list of options available.
    void setOption(Option opt);

    // Get information about the current process.
    // Returns bitwise-or'd zygisk::StateFlag values.
    uint32_t getFlags();

    // Exempt the provided file descriptor from being automatically closed.
    //
    // This API only make sense in preAppSpecialize; calling this method in any other situation
    // is either a no-op (returns true) or an error (returns false).
    //
    // When false is returned, the provided file descriptor will eventually be closed by zygote.
    bool exemptFd(int fd);

    // Hook JNI native methods for a class
    //
    // Lookup all registered JNI native methods and replace it with your own methods.
    // The original function pointer will be saved in each JNINativeMethod's fnPtr.
    // If no matching class, method name, or signature is found, that specific JNINativeMethod.fnPtr
    // will be set to nullptr.
    void hookJniNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *methods, int numMethods);

    // Hook functions in the PLT (Procedure Linkage Table) of ELFs loaded in memory.
    //
    // Parsing /proc/[PID]/maps will give you the memory map of a process. As an example:
    //
    //       <address>       <perms>  <offset>   <dev>  <inode>           <pathname>
    // 56b4346000-56b4347000  r-xp    00002000   fe:00    235       /system/bin/app_process64
    // (More details: https://man7.org/linux/man-pages/man5/proc.5.html)
    //
    // The `dev` and `inode` pair uniquely identifies a file being mapped into memory.
    // For matching ELFs loaded in memory, replace function `symbol` with `newFunc`.
    // If `oldFunc` is not nullptr, the original function pointer will be saved to `oldFunc`.
    void pltHookRegister(dev_t dev, ino_t inode, const char *symbol, void *newFunc, void **oldFunc);

    // Commit all the hooks that was previously registered.
    // Returns false if an error occurred.
    bool pltHookCommit();

private:
    internal::api_table *tbl;
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
// which is a Unix domain socket that is connected to the target process.
// See Api::connectCompanion() for more info.
//
// NOTE: the function can run concurrently on multiple threads.
// Be aware of race conditions if you have globally shared resources.

#define REGISTER_ZYGISK_COMPANION(func) \
void zygisk_companion_entry(int client) { func(client); }

/*********************************************************
 * The following is internal ABI implementation detail.
 * You do not have to understand what it is doing.
 *********************************************************/

namespace internal {

struct module_abi {
    long api_version;
    ModuleBase *impl;

    void (*preAppSpecialize)(ModuleBase *, AppSpecializeArgs *);
    void (*postAppSpecialize)(ModuleBase *, const AppSpecializeArgs *);
    void (*preServerSpecialize)(ModuleBase *, ServerSpecializeArgs *);
    void (*postServerSpecialize)(ModuleBase *, const ServerSpecializeArgs *);

    module_abi(ModuleBase *module) : api_version(ZYGISK_API_VERSION), impl(module) {
        preAppSpecialize = [](auto m, auto args) { m->preAppSpecialize(args); };
        postAppSpecialize = [](auto m, auto args) { m->postAppSpecialize(args); };
        preServerSpecialize = [](auto m, auto args) { m->preServerSpecialize(args); };
        postServerSpecialize = [](auto m, auto args) { m->postServerSpecialize(args); };
    }
};

struct api_table {
    // Base
    void *impl;
    bool (*registerModule)(api_table *, module_abi *);

    void (*hookJniNativeMethods)(JNIEnv *, const char *, JNINativeMethod *, int);
    void (*pltHookRegister)(dev_t, ino_t, const char *, void *, void **);
    bool (*exemptFd)(int);
    bool (*pltHookCommit)();
    int  (*connectCompanion)(void * /* impl */);
    void (*setOption)(void * /* impl */, Option);
    int  (*getModuleDir)(void * /* impl */);
    uint32_t (*getFlags)(void * /* impl */);
};

template <class T>
void entry_impl(api_table *table, JNIEnv *env) {
    static Api api;
    api.tbl = table;
    static T module;
    ModuleBase *m = &module;
    static module_abi abi(m);
    if (!table->registerModule(table, &abi)) return;
    m->onLoad(&api, env);
}

} // namespace internal

inline int Api::connectCompanion() {
    return tbl->connectCompanion ? tbl->connectCompanion(tbl->impl) : -1;
}
inline int Api::getModuleDir() {
    return tbl->getModuleDir ? tbl->getModuleDir(tbl->impl) : -1;
}
inline void Api::setOption(Option opt) {
    if (tbl->setOption) tbl->setOption(tbl->impl, opt);
}
inline uint32_t Api::getFlags() {
    return tbl->getFlags ? tbl->getFlags(tbl->impl) : 0;
}
inline bool Api::exemptFd(int fd) {
    return tbl->exemptFd != nullptr && tbl->exemptFd(fd);
}
inline void Api::hookJniNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *methods, int numMethods) {
    if (tbl->hookJniNativeMethods) tbl->hookJniNativeMethods(env, className, methods, numMethods);
}
inline void Api::pltHookRegister(dev_t dev, ino_t inode, const char *symbol, void *newFunc, void **oldFunc) {
    if (tbl->pltHookRegister) tbl->pltHookRegister(dev, inode, symbol, newFunc, oldFunc);
}
inline bool Api::pltHookCommit() {
    return tbl->pltHookCommit != nullptr && tbl->pltHookCommit();
}

} // namespace zygisk

extern "C" {

[[gnu::visibility("default"), maybe_unused]]
void zygisk_module_entry(zygisk::internal::api_table *, JNIEnv *);

[[gnu::visibility("default"), maybe_unused]]
void zygisk_companion_entry(int);

} // extern "C"
