/*
 * Copyright (C) 2007 The Android Open Source Project
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

#include <dlfcn.h>
#include <link.h>
#include <stdlib.h>
#include <android/dlext.h>

// These functions are exported by the loader
// TODO(dimitry): replace these with reference to libc.so

extern "C" {

__attribute__((__weak__, visibility("default")))
void __loader_android_get_LD_LIBRARY_PATH(char* buffer, size_t buffer_size);

__attribute__((__weak__, visibility("default")))
void __loader_android_update_LD_LIBRARY_PATH(const char* ld_library_path);

__attribute__((__weak__, visibility("default")))
void __loader_android_set_application_target_sdk_version(int target);

__attribute__((__weak__, visibility("default")))
bool __loader_android_init_anonymous_namespace(const char* shared_libs_sonames,
                                               const char* library_search_path);

__attribute__((__weak__, visibility("default")))
struct android_namespace_t* __loader_android_create_namespace(
                                const char* name,
                                const char* ld_library_path,
                                const char* default_library_path,
                                uint64_t type,
                                const char* permitted_when_isolated_path,
                                struct android_namespace_t* parent,
                                const void* caller_addr);

__attribute__((__weak__, visibility("default")))
bool __loader_android_link_namespaces(
                                struct android_namespace_t* namespace_from,
                                struct android_namespace_t* namespace_to,
                                const char* shared_libs_sonames);

__attribute__((__weak__, visibility("default")))
void __loader_android_dlwarning(void* obj, void (*f)(void*, const char*));

__attribute__((__weak__, visibility("default")))
struct android_namespace_t* __loader_android_get_exported_namespace(const char* name);

// Proxy calls to bionic loader
__attribute__((__weak__))
void android_get_LD_LIBRARY_PATH(char* buffer, size_t buffer_size) {
  __loader_android_get_LD_LIBRARY_PATH(buffer, buffer_size);
}

__attribute__((__weak__))
void android_update_LD_LIBRARY_PATH(const char* ld_library_path) {
  __loader_android_update_LD_LIBRARY_PATH(ld_library_path);
}

__attribute__((__weak__))
void android_set_application_target_sdk_version(int target) {
  __loader_android_set_application_target_sdk_version(target);
}

__attribute__((__weak__))
bool android_init_anonymous_namespace(const char* shared_libs_sonames,
                                      const char* library_search_path) {
  return __loader_android_init_anonymous_namespace(shared_libs_sonames, library_search_path);
}

__attribute__((__weak__))
struct android_namespace_t* android_create_namespace(const char* name,
                                                     const char* ld_library_path,
                                                     const char* default_library_path,
                                                     uint64_t type,
                                                     const char* permitted_when_isolated_path,
                                                     struct android_namespace_t* parent) {
  const void* caller_addr = __builtin_return_address(0);
  return __loader_android_create_namespace(name,
                                           ld_library_path,
                                           default_library_path,
                                           type,
                                           permitted_when_isolated_path,
                                           parent,
                                           caller_addr);
}

__attribute__((__weak__))
bool android_link_namespaces(struct android_namespace_t* namespace_from,
                             struct android_namespace_t* namespace_to,
                             const char* shared_libs_sonames) {
  return __loader_android_link_namespaces(namespace_from, namespace_to, shared_libs_sonames);
}

__attribute__((__weak__))
void android_dlwarning(void* obj, void (*f)(void*, const char*)) {
  __loader_android_dlwarning(obj, f);
}

__attribute__((__weak__))
struct android_namespace_t* android_get_exported_namespace(const char* name) {
  return __loader_android_get_exported_namespace(name);
}

} // extern "C"
