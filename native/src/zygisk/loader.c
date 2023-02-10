#include <dlfcn.h>
#include <android/dlext.h>

#if defined(__LP64__)
// Use symlink to workaround linker bug on old broken Android
// https://issuetracker.google.com/issues/36914295
#define SECOND_STAGE_PATH "/system/bin/app_process"
#else
#define SECOND_STAGE_PATH "/system/bin/app_process32"
#endif

__attribute__((constructor))
static void zygisk_loader() {
    android_dlextinfo info = {
        .flags = ANDROID_DLEXT_FORCE_LOAD
    };
    // Android 5.x doesn't support ANDROID_DLEXT_FORCE_LOAD
    void *handle =
            android_dlopen_ext(SECOND_STAGE_PATH, RTLD_LAZY, &info) ?:
            dlopen(SECOND_STAGE_PATH, RTLD_LAZY);
    if (handle) {
        void(*entry)(void*) = dlsym(handle, "zygisk_inject_entry");
        if (entry) {
            entry(handle);
        }
    }
}
