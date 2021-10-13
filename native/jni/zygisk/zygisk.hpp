#pragma once

#include <stdint.h>
#include <jni.h>
#include <vector>

#define INJECT_ENV_1 "MAGISK_INJ_1"
#define INJECT_ENV_2 "MAGISK_INJ_2"

enum : int {
    ZYGISK_SETUP,
    ZYGISK_GET_INFO,
    ZYGISK_UNMOUNT,
    ZYGISK_GET_LOG_PIPE,
};

// Unmap all pages matching the name
void unmap_all(const char *name);

// Remap all matching pages with anonymous pages
void remap_all(const char *name);

// Get library name + offset (from start of ELF), given function address
uintptr_t get_function_off(int pid, uintptr_t addr, char *lib);

// Get function address, given library name + offset
uintptr_t get_function_addr(int pid, const char *lib, uintptr_t off);

struct AppInfo {
    bool is_magisk_app;
    bool on_denylist;
};

void self_unload();
void hook_functions();
bool unhook_functions();
std::vector<int> remote_get_info(int uid, const char *process, AppInfo *info);
int remote_request_unmount();
