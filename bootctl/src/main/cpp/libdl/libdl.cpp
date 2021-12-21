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
void* __loader_dlopen(const char* filename, int flags, const void* caller_addr);

__attribute__((__weak__, visibility("default")))
char* __loader_dlerror();

__attribute__((__weak__, visibility("default")))
void* __loader_dlsym(void* handle, const char* symbol, const void* caller_addr);

__attribute__((__weak__, visibility("default")))
void* __loader_dlvsym(void* handle,
                      const char* symbol,
                      const char* version,
                      const void* caller_addr);

__attribute__((__weak__, visibility("default")))
int __loader_dladdr(const void* addr, Dl_info* info);

__attribute__((__weak__, visibility("default")))
int __loader_dlclose(void* handle);

#if defined(__arm__)
__attribute__((__weak__, visibility("default")))
_Unwind_Ptr __loader_dl_unwind_find_exidx(_Unwind_Ptr pc, int* pcount);
#endif

__attribute__((__weak__, visibility("default")))
int __loader_dl_iterate_phdr(int (*cb)(struct dl_phdr_info* info, size_t size, void* data),
                             void* data);

__attribute__((__weak__, visibility("default")))
void __loader_android_get_LD_LIBRARY_PATH(char* buffer, size_t buffer_size);

__attribute__((__weak__, visibility("default")))
void __loader_android_update_LD_LIBRARY_PATH(const char* ld_library_path);

__attribute__((__weak__, visibility("default")))
void* __loader_android_dlopen_ext(const char* filename,
                                  int flag,
                                  const android_dlextinfo* extinfo,
                                  const void* caller_addr);

__attribute__((__weak__, visibility("default")))
int __loader_android_get_application_target_sdk_version();

// Proxy calls to bionic loader
__attribute__((__weak__))
void android_get_LD_LIBRARY_PATH(char* buffer, size_t buffer_size) {
  __loader_android_get_LD_LIBRARY_PATH(buffer, buffer_size);
}

__attribute__((__weak__))
void* dlopen(const char* filename, int flag) {
  const void* caller_addr = __builtin_return_address(0);
  return __loader_dlopen(filename, flag, caller_addr);
}

__attribute__((__weak__))
char* dlerror() {
  return __loader_dlerror();
}

__attribute__((__weak__))
void* dlsym(void* handle, const char* symbol) {
  const void* caller_addr = __builtin_return_address(0);
  return __loader_dlsym(handle, symbol, caller_addr);
}

__attribute__((__weak__))
void* dlvsym(void* handle, const char* symbol, const char* version) {
  const void* caller_addr = __builtin_return_address(0);
  return __loader_dlvsym(handle, symbol, version, caller_addr);
}

__attribute__((__weak__))
int dladdr(const void* addr, Dl_info* info) {
  return __loader_dladdr(addr, info);
}

__attribute__((__weak__))
int dlclose(void* handle) {
  return __loader_dlclose(handle);
}

#if defined(__arm__)
__attribute__((__weak__))
_Unwind_Ptr dl_unwind_find_exidx(_Unwind_Ptr pc, int* pcount) {
  return __loader_dl_unwind_find_exidx(pc, pcount);
}
#endif

/*
 * This needs to be defined as weak because it is also defined in libc.a.
 * Without this, static executables will have a multiple definition error.
 */
__attribute__((__weak__))
int dl_iterate_phdr(int (*cb)(struct dl_phdr_info* info, size_t size, void* data), void* data) {
  return __loader_dl_iterate_phdr(cb, data);
}

__attribute__((__weak__))
void* android_dlopen_ext(const char* filename, int flag, const android_dlextinfo* extinfo) {
  const void* caller_addr = __builtin_return_address(0);
  return __loader_android_dlopen_ext(filename, flag, extinfo, caller_addr);
}

__attribute__((__weak__))
int android_get_application_target_sdk_version() {
  return __loader_android_get_application_target_sdk_version();
}

} // extern "C"
