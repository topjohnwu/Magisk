#include <dlfcn.h>
#include <android/dlext.h>

#if defined(__LP64__)
#define SECOND_STAGE_PATH "/system/bin/magisk64"
#else
#define SECOND_STAGE_PATH "/system/bin/magisk32"
#endif

__attribute__((constructor))
static void zygisk_loader() {
    android_dlextinfo info = {
        .flags = ANDROID_DLEXT_FORCE_LOAD
    };
    void *handle = android_dlopen_ext(SECOND_STAGE_PATH, RTLD_LAZY, &info);
    if (handle) {
        void(*entry)(void*) = dlsym(handle, "zygisk_inject_entry");
        if (entry) {
            entry(handle);
        }
    }
}
