#pragma once

#include <stdint.h>
#include <jni.h>

#define INJECT_LIB_1 "/dev/tmp/magisk.1.so"
#define INJECT_LIB_2 "/dev/tmp/magisk.2.so"
#define INJECT_ENV_1 "MAGISK_INJ_1"
#define INJECT_ENV_2 "MAGISK_INJ_2"

// Unmap all pages matching the name
void unmap_all(const char *name);

// Get library name and base address that contains the function
uintptr_t get_function_lib(uintptr_t addr, char *lib);

// Get library base address with name
uintptr_t get_remote_lib(int pid, const char *lib);

void self_unload();
void hook_functions();
bool unhook_functions();
