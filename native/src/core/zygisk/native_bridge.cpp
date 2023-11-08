#include <android/dlext.h>
#include <dlfcn.h>
#include <unwind.h>

#include <consts.hpp>
#include <core.hpp>

#include "zygisk.hpp"

static bool is_compatible_with(uint32_t) {
    auto name = get_prop(NBPROP);
    android_dlextinfo info = {
        .flags = ANDROID_DLEXT_FORCE_LOAD
    };
    void *handle = android_dlopen_ext(name.data(), RTLD_LAZY, &info);
    if (handle) {
        auto entry = reinterpret_cast<void (*)(void *)>(dlsym(handle, "zygisk_inject_entry"));
        if (entry) {
            entry(handle);
        }
    }
    return false;
}

extern "C" [[maybe_unused]] NativeBridgeCallbacks NativeBridgeItf{
    .version = 2,
    .padding = {},
    .isCompatibleWith = &is_compatible_with,
};
