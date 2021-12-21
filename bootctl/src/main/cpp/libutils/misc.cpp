/*
 * Copyright (C) 2005 The Android Open Source Project
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

#define LOG_TAG "misc"

#include <utils/misc.h>

#include <pthread.h>

#include <utils/Log.h>
#include <utils/Vector.h>

#if defined(__ANDROID__) && !defined(__ANDROID_RECOVERY__)
#include <dlfcn.h>
#include <vndksupport/linker.h>
#endif

extern "C" void do_report_sysprop_change();

using namespace android;

namespace android {

struct sysprop_change_callback_info {
    sysprop_change_callback callback;
    int priority;
};

#if !defined(_WIN32)
static pthread_mutex_t gSyspropMutex = PTHREAD_MUTEX_INITIALIZER;
static Vector<sysprop_change_callback_info>* gSyspropList = nullptr;
#endif

#if !defined(_WIN32)
void add_sysprop_change_callback(sysprop_change_callback cb, int priority) {
    pthread_mutex_lock(&gSyspropMutex);
    if (gSyspropList == nullptr) {
        gSyspropList = new Vector<sysprop_change_callback_info>();
    }
    sysprop_change_callback_info info;
    info.callback = cb;
    info.priority = priority;
    bool added = false;
    for (size_t i=0; i<gSyspropList->size(); i++) {
        if (priority >= gSyspropList->itemAt(i).priority) {
            gSyspropList->insertAt(info, i);
            added = true;
            break;
        }
    }
    if (!added) {
        gSyspropList->add(info);
    }
    pthread_mutex_unlock(&gSyspropMutex);
}
#else
void add_sysprop_change_callback(sysprop_change_callback, int) {}
#endif

#if defined(__ANDROID__) && !defined(__ANDROID_RECOVERY__)
void (*get_report_sysprop_change_func())() {
    void (*func)() = nullptr;
    void* handle = android_load_sphal_library("libutils.so", RTLD_NOW);
    if (handle != nullptr) {
        func = reinterpret_cast<decltype(func)>(dlsym(handle, "do_report_sysprop_change"));
    }

    return func;
}
#endif

void report_sysprop_change() {
    do_report_sysprop_change();

#if defined(__ANDROID__) && !defined(__ANDROID_RECOVERY__)
    // libutils.so is double loaded; from the default namespace and from the
    // 'sphal' namespace. Redirect the sysprop change event to the other instance
    // of libutils.so loaded in the 'sphal' namespace so that listeners attached
    // to that instance is also notified with this event.
    static auto func = get_report_sysprop_change_func();
    if (func != nullptr) {
        (*func)();
    }
#endif
}

};  // namespace android

void do_report_sysprop_change() {
#if !defined(_WIN32)
    pthread_mutex_lock(&gSyspropMutex);
    Vector<sysprop_change_callback_info> listeners;
    if (gSyspropList != nullptr) {
        listeners = *gSyspropList;
    }
    pthread_mutex_unlock(&gSyspropMutex);

    //ALOGI("Reporting sysprop change to %d listeners", listeners.size());
    for (size_t i=0; i<listeners.size(); i++) {
        listeners[i].callback();
    }
#endif
}
