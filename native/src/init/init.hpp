#pragma once

#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"
#define REDIR_PATH "/data/magiskinit"

#define PRELOAD_LIB    "/dev/preload.so"
#define PRELOAD_POLICY "/dev/sepolicy"
#define PRELOAD_ACK    "/dev/ack"

#ifdef __cplusplus

#include <base.hpp>
#include <sepolicy.hpp>

#include "init-rs.hpp"

int magisk_proxy_main(int, char *argv[]);
rust::Utf8CStr backup_init();

// Expose some constants to Rust

static inline rust::Utf8CStr split_plat_cil() {
    return SPLIT_PLAT_CIL;
};

static inline rust::Utf8CStr preload_lib() {
    return PRELOAD_LIB;
}

static inline rust::Utf8CStr preload_policy() {
    return PRELOAD_POLICY;
}

static inline rust::Utf8CStr preload_ack() {
    return PRELOAD_ACK;
}


#endif
