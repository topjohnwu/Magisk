/*
 * Copyright (C) 2017 The Android Open Source Project
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

#define LOG_TAG "vndksupport"

#include "linker.h"

#include <android/dlext.h>
#include <dlfcn.h>
#include <log/log.h>
#include <sys/types.h>
#include <unistd.h>

#include <initializer_list>

extern "C" android_namespace_t* android_get_exported_namespace(const char*);

namespace {

struct VendorNamespace {
    android_namespace_t* ptr = nullptr;
    const char* name = nullptr;
};

}  // anonymous namespace

static VendorNamespace get_vendor_namespace() {
    static VendorNamespace result = ([] {
        for (const char* name : {"sphal", "default"}) {
            if (android_namespace_t* ns = android_get_exported_namespace(name)) {
                return VendorNamespace{ns, name};
            }
        }
        return VendorNamespace{};
    })();
    return result;
}

int android_is_in_vendor_process() {
    // Special case init, since when init runs, ld.config.<ver>.txt hasn't been
    // loaded (sysprop service isn't up for init to know <ver>).
    if (getpid() == 1) {
        return 0;
    }

    // In vendor process, 'vndk' namespace is not visible, whereas in system
    // process, it is.
    return android_get_exported_namespace("vndk") == nullptr;
}

void* android_load_sphal_library(const char* name, int flag) {
    VendorNamespace vendor_namespace = get_vendor_namespace();
    if (vendor_namespace.ptr != nullptr) {
        const android_dlextinfo dlextinfo = {
                .flags = ANDROID_DLEXT_USE_NAMESPACE,
                .library_namespace = vendor_namespace.ptr,
        };
        void* handle = android_dlopen_ext(name, flag, &dlextinfo);
        if (!handle) {
            ALOGE("Could not load %s from %s namespace: %s.", name, vendor_namespace.name,
                  dlerror());
        }
        return handle;
    } else {
        ALOGD("Loading %s from current namespace instead of sphal namespace.", name);
        return dlopen(name, flag);
    }
}

int android_unload_sphal_library(void* handle) {
    return dlclose(handle);
}
