/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ART_LIBNATIVEBRIDGE_INCLUDE_NATIVEBRIDGE_NATIVE_BRIDGE_H_
#define ART_LIBNATIVEBRIDGE_INCLUDE_NATIVEBRIDGE_NATIVE_BRIDGE_H_

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "jni.h"

#ifdef __cplusplus
namespace android {
extern "C" {
#endif  // __cplusplus

// Loads a shared library from the system linker namespace, suitable for
// platform libraries in /system/lib(64). If linker namespaces don't exist (i.e.
// on host), this simply calls dlopen().
void* OpenSystemLibrary(const char* path, int flags);

struct NativeBridgeRuntimeCallbacks;
struct NativeBridgeRuntimeValues;

// Function pointer type for sigaction. This is mostly the signature of a signal handler, except
// for the return type. The runtime needs to know whether the signal was handled or should be given
// to the chain.
typedef bool (*NativeBridgeSignalHandlerFn)(int, siginfo_t*, void*);  // NOLINT

// Open the native bridge, if any. Should be called by Runtime::Init(). A null library filename
// signals that we do not want to load a native bridge.
bool LoadNativeBridge(const char* native_bridge_library_filename,
                      const struct NativeBridgeRuntimeCallbacks* runtime_callbacks);

// Quick check whether a native bridge will be needed. This is based off of the instruction set
// of the process.
bool NeedsNativeBridge(const char* instruction_set);

// Do the early initialization part of the native bridge, if necessary. This should be done under
// high privileges.
bool PreInitializeNativeBridge(const char* app_data_dir, const char* instruction_set);

// Prepare to fork from zygote. May be required to clean-up the enviroment, e.g.
// close emulated file descriptors, after doPreload() in app-zygote.
void PreZygoteForkNativeBridge();

// Initialize the native bridge, if any. Should be called by Runtime::DidForkFromZygote. The JNIEnv*
// will be used to modify the app environment for the bridge.
bool InitializeNativeBridge(JNIEnv* env, const char* instruction_set);

// Unload the native bridge, if any. Should be called by Runtime::DidForkFromZygote.
void UnloadNativeBridge();

// Check whether a native bridge is available (opened or initialized). Requires a prior call to
// LoadNativeBridge.
bool NativeBridgeAvailable();

// Check whether a native bridge is available (initialized). Requires a prior call to
// LoadNativeBridge & InitializeNativeBridge.
bool NativeBridgeInitialized();

// Load a shared library that is supported by the native bridge.
//
// Starting with v3, NativeBridge has two scenarios: with/without namespace.
// Use NativeBridgeLoadLibraryExt() instead in namespace scenario.
void* NativeBridgeLoadLibrary(const char* libpath, int flag);

// Get a native bridge trampoline for specified native method.
void* NativeBridgeGetTrampoline(void* handle, const char* name, const char* shorty, uint32_t len);

// True if native library paths are valid and is for an ABI that is supported by native bridge.
// The *libpath* must point to a library.
//
// Starting with v3, NativeBridge has two scenarios: with/without namespace.
// Use NativeBridgeIsPathSupported() instead in namespace scenario.
bool NativeBridgeIsSupported(const char* libpath);

// Returns the version number of the native bridge. This information is available after a
// successful LoadNativeBridge() and before closing it, that is, as long as NativeBridgeAvailable()
// returns true. Returns 0 otherwise.
uint32_t NativeBridgeGetVersion();

// Returns a signal handler that the bridge would like to be managed. Only valid for a native
// bridge supporting the version 2 interface. Will return null if the bridge does not support
// version 2, or if it doesn't have a signal handler it wants to be known.
NativeBridgeSignalHandlerFn NativeBridgeGetSignalHandler(int signal);

// Returns whether we have seen a native bridge error. This could happen because the library
// was not found, rejected, could not be initialized and so on.
//
// This functionality is mainly for testing.
bool NativeBridgeError();

// Returns whether a given string is acceptable as a native bridge library filename.
//
// This functionality is exposed mainly for testing.
bool NativeBridgeNameAcceptable(const char* native_bridge_library_filename);

// Decrements the reference count on the dynamic library handler. If the reference count drops
// to zero then the dynamic library is unloaded. Returns 0 on success and non-zero on error.
int NativeBridgeUnloadLibrary(void* handle);

// Get last error message of native bridge when fail to load library or search symbol.
// This is reflection of dlerror() for native bridge.
const char* NativeBridgeGetError();

struct native_bridge_namespace_t;

// True if native library paths are valid and is for an ABI that is supported by native bridge.
// Different from NativeBridgeIsSupported(), the *path* here must be a directory containing
// libraries of an ABI.
//
// Starting with v3, NativeBridge has two scenarios: with/without namespace.
// Use NativeBridgeIsSupported() instead in non-namespace scenario.
bool NativeBridgeIsPathSupported(const char* path);

// Initializes anonymous namespace.
// NativeBridge's peer of android_init_anonymous_namespace() of dynamic linker.
//
// The anonymous namespace is used in the case when a NativeBridge implementation
// cannot identify the caller of dlopen/dlsym which happens for the code not loaded
// by dynamic linker; for example calls from the mono-compiled code.
//
// Starting with v3, NativeBridge has two scenarios: with/without namespace.
// Should not use in non-namespace scenario.
bool NativeBridgeInitAnonymousNamespace(const char* public_ns_sonames,
                                        const char* anon_ns_library_path);

// Create new namespace in which native libraries will be loaded.
// NativeBridge's peer of android_create_namespace() of dynamic linker.
//
// The libraries in the namespace are searched by folowing order:
// 1. ld_library_path (Think of this as namespace-local LD_LIBRARY_PATH)
// 2. In directories specified by DT_RUNPATH of the "needed by" binary.
// 3. deault_library_path (This of this as namespace-local default library path)
//
// Starting with v3, NativeBridge has two scenarios: with/without namespace.
// Should not use in non-namespace scenario.
struct native_bridge_namespace_t* NativeBridgeCreateNamespace(
    const char* name, const char* ld_library_path, const char* default_library_path, uint64_t type,
    const char* permitted_when_isolated_path, struct native_bridge_namespace_t* parent_ns);

// Creates a link which shares some libraries from one namespace to another.
// NativeBridge's peer of android_link_namespaces() of dynamic linker.
//
// Starting with v3, NativeBridge has two scenarios: with/without namespace.
// Should not use in non-namespace scenario.
bool NativeBridgeLinkNamespaces(struct native_bridge_namespace_t* from,
                                struct native_bridge_namespace_t* to,
                                const char* shared_libs_sonames);

// Load a shared library with namespace key that is supported by the native bridge.
// NativeBridge's peer of android_dlopen_ext() of dynamic linker, only supports namespace
// extension.
//
// Starting with v3, NativeBridge has two scenarios: with/without namespace.
// Use NativeBridgeLoadLibrary() instead in non-namespace scenario.
void* NativeBridgeLoadLibraryExt(const char* libpath, int flag,
                                 struct native_bridge_namespace_t* ns);

// Returns exported namespace by the name. This is a reflection of
// android_get_exported_namespace function. Introduced in v5.
struct native_bridge_namespace_t* NativeBridgeGetExportedNamespace(const char* name);

// Native bridge interfaces to runtime.
struct NativeBridgeCallbacks {
  // Version number of the interface.
  uint32_t version;

  // Initialize native bridge. Native bridge's internal implementation must ensure MT safety and
  // that the native bridge is initialized only once. Thus it is OK to call this interface for an
  // already initialized native bridge.
  //
  // Parameters:
  //   runtime_cbs [IN] the pointer to NativeBridgeRuntimeCallbacks.
  // Returns:
  //   true if initialization was successful.
  bool (*initialize)(const struct NativeBridgeRuntimeCallbacks* runtime_cbs,
                     const char* private_dir, const char* instruction_set);

  // Load a shared library that is supported by the native bridge.
  //
  // Parameters:
  //   libpath [IN] path to the shared library
  //   flag [IN] the stardard RTLD_XXX defined in bionic dlfcn.h
  // Returns:
  //   The opaque handle of the shared library if sucessful, otherwise NULL
  //
  // Starting with v3, NativeBridge has two scenarios: with/without namespace.
  // Use loadLibraryExt instead in namespace scenario.
  void* (*loadLibrary)(const char* libpath, int flag);

  // Get a native bridge trampoline for specified native method. The trampoline has same
  // sigature as the native method.
  //
  // Parameters:
  //   handle [IN] the handle returned from loadLibrary
  //   shorty [IN] short descriptor of native method
  //   len [IN] length of shorty
  // Returns:
  //   address of trampoline if successful, otherwise NULL
  void* (*getTrampoline)(void* handle, const char* name, const char* shorty, uint32_t len);

  // Check whether native library is valid and is for an ABI that is supported by native bridge.
  //
  // Parameters:
  //   libpath [IN] path to the shared library
  // Returns:
  //   TRUE if library is supported by native bridge, FALSE otherwise
  //
  // Starting with v3, NativeBridge has two scenarios: with/without namespace.
  // Use isPathSupported instead in namespace scenario.
  bool (*isSupported)(const char* libpath);

  // Provide environment values required by the app running with native bridge according to the
  // instruction set.
  //
  // Parameters:
  //   instruction_set [IN] the instruction set of the app
  // Returns:
  //   NULL if not supported by native bridge.
  //   Otherwise, return all environment values to be set after fork.
  const struct NativeBridgeRuntimeValues* (*getAppEnv)(const char* instruction_set);

  // Added callbacks in version 2.

  // Check whether the bridge is compatible with the given version. A bridge may decide not to be
  // forwards- or backwards-compatible, and libnativebridge will then stop using it.
  //
  // Parameters:
  //   bridge_version [IN] the version of libnativebridge.
  // Returns:
  //   true if the native bridge supports the given version of libnativebridge.
  bool (*isCompatibleWith)(uint32_t bridge_version);

  // A callback to retrieve a native bridge's signal handler for the specified signal. The runtime
  // will ensure that the signal handler is being called after the runtime's own handler, but before
  // all chained handlers. The native bridge should not try to install the handler by itself, as
  // that will potentially lead to cycles.
  //
  // Parameters:
  //   signal [IN] the signal for which the handler is asked for. Currently, only SIGSEGV is
  //                 supported by the runtime.
  // Returns:
  //   NULL if the native bridge doesn't use a handler or doesn't want it to be managed by the
  //   runtime.
  //   Otherwise, a pointer to the signal handler.
  NativeBridgeSignalHandlerFn (*getSignalHandler)(int signal);

  // Added callbacks in version 3.

  // Decrements the reference count on the dynamic library handler. If the reference count drops
  // to zero then the dynamic library is unloaded.
  //
  // Parameters:
  //   handle [IN] the handler of a dynamic library.
  //
  // Returns:
  //   0 on success, and nonzero on error.
  int (*unloadLibrary)(void* handle);

  // Dump the last failure message of native bridge when fail to load library or search symbol.
  //
  // Parameters:
  //
  // Returns:
  //   A string describing the most recent error that occurred when load library
  //   or lookup symbol via native bridge.
  const char* (*getError)();

  // Check whether library paths are supported by native bridge.
  //
  // Parameters:
  //   library_path [IN] search paths for native libraries (directories separated by ':')
  // Returns:
  //   TRUE if libraries within search paths are supported by native bridge, FALSE otherwise
  //
  // Starting with v3, NativeBridge has two scenarios: with/without namespace.
  // Use isSupported instead in non-namespace scenario.
  bool (*isPathSupported)(const char* library_path);

  // Initializes anonymous namespace at native bridge side.
  // NativeBridge's peer of android_init_anonymous_namespace() of dynamic linker.
  //
  // The anonymous namespace is used in the case when a NativeBridge implementation
  // cannot identify the caller of dlopen/dlsym which happens for the code not loaded
  // by dynamic linker; for example calls from the mono-compiled code.
  //
  // Parameters:
  //   public_ns_sonames [IN] the name of "public" libraries.
  //   anon_ns_library_path [IN] the library search path of (anonymous) namespace.
  // Returns:
  //   true if the pass is ok.
  //   Otherwise, false.
  //
  // Starting with v3, NativeBridge has two scenarios: with/without namespace.
  // Should not use in non-namespace scenario.
  bool (*initAnonymousNamespace)(const char* public_ns_sonames, const char* anon_ns_library_path);

  // Create new namespace in which native libraries will be loaded.
  // NativeBridge's peer of android_create_namespace() of dynamic linker.
  //
  // Parameters:
  //   name [IN] the name of the namespace.
  //   ld_library_path [IN] the first set of library search paths of the namespace.
  //   default_library_path [IN] the second set of library search path of the namespace.
  //   type [IN] the attribute of the namespace.
  //   permitted_when_isolated_path [IN] the permitted path for isolated namespace(if it is).
  //   parent_ns [IN] the pointer of the parent namespace to be inherited from.
  // Returns:
  //   native_bridge_namespace_t* for created namespace or nullptr in the case of error.
  //
  // Starting with v3, NativeBridge has two scenarios: with/without namespace.
  // Should not use in non-namespace scenario.
  struct native_bridge_namespace_t* (*createNamespace)(const char* name,
                                                       const char* ld_library_path,
                                                       const char* default_library_path,
                                                       uint64_t type,
                                                       const char* permitted_when_isolated_path,
                                                       struct native_bridge_namespace_t* parent_ns);

  // Creates a link which shares some libraries from one namespace to another.
  // NativeBridge's peer of android_link_namespaces() of dynamic linker.
  //
  // Parameters:
  //   from [IN] the namespace where libraries are accessed.
  //   to [IN] the namespace where libraries are loaded.
  //   shared_libs_sonames [IN] the libraries to be shared.
  //
  // Returns:
  //   Whether successed or not.
  //
  // Starting with v3, NativeBridge has two scenarios: with/without namespace.
  // Should not use in non-namespace scenario.
  bool (*linkNamespaces)(struct native_bridge_namespace_t* from,
                         struct native_bridge_namespace_t* to, const char* shared_libs_sonames);

  // Load a shared library within a namespace.
  // NativeBridge's peer of android_dlopen_ext() of dynamic linker, only supports namespace
  // extension.
  //
  // Parameters:
  //   libpath [IN] path to the shared library
  //   flag [IN] the stardard RTLD_XXX defined in bionic dlfcn.h
  //   ns [IN] the pointer of the namespace in which the library should be loaded.
  // Returns:
  //   The opaque handle of the shared library if sucessful, otherwise NULL
  //
  // Starting with v3, NativeBridge has two scenarios: with/without namespace.
  // Use loadLibrary instead in non-namespace scenario.
  void* (*loadLibraryExt)(const char* libpath, int flag, struct native_bridge_namespace_t* ns);

  // Get native bridge version of vendor namespace.
  // The vendor namespace is the namespace used to load vendor public libraries.
  // With O release this namespace can be different from the default namespace.
  // For the devices without enable vendor namespaces this function should return null
  //
  // Returns:
  //   vendor namespace or null if it was not set up for the device
  //
  // Starting with v5 (Android Q) this function is no longer used.
  // Use getExportedNamespace() below.
  struct native_bridge_namespace_t* (*getVendorNamespace)();

  // Get native bridge version of exported namespace. Peer of
  // android_get_exported_namespace(const char*) function.
  //
  // Returns:
  //   exported namespace or null if it was not set up for the device
  struct native_bridge_namespace_t* (*getExportedNamespace)(const char* name);

  // If native bridge is used in app-zygote (in doPreload()) this callback is
  // required to clean-up the environment before the fork (see b/146904103).
  void (*preZygoteFork)();
};

// Runtime interfaces to native bridge.
struct NativeBridgeRuntimeCallbacks {
  // Get shorty of a Java method. The shorty is supposed to be persistent in memory.
  //
  // Parameters:
  //   env [IN] pointer to JNIenv.
  //   mid [IN] Java methodID.
  // Returns:
  //   short descriptor for method.
  const char* (*getMethodShorty)(JNIEnv* env, jmethodID mid);

  // Get number of native methods for specified class.
  //
  // Parameters:
  //   env [IN] pointer to JNIenv.
  //   clazz [IN] Java class object.
  // Returns:
  //   number of native methods.
  uint32_t (*getNativeMethodCount)(JNIEnv* env, jclass clazz);

  // Get at most 'method_count' native methods for specified class 'clazz'. Results are outputed
  // via 'methods' [OUT]. The signature pointer in JNINativeMethod is reused as the method shorty.
  //
  // Parameters:
  //   env [IN] pointer to JNIenv.
  //   clazz [IN] Java class object.
  //   methods [OUT] array of method with the name, shorty, and fnPtr.
  //   method_count [IN] max number of elements in methods.
  // Returns:
  //   number of method it actually wrote to methods.
  uint32_t (*getNativeMethods)(JNIEnv* env, jclass clazz, JNINativeMethod* methods,
                               uint32_t method_count);
};

#ifdef __cplusplus
}  // extern "C"
}  // namespace android
#endif  // __cplusplus

#endif  // ART_LIBNATIVEBRIDGE_INCLUDE_NATIVEBRIDGE_NATIVE_BRIDGE_H_
