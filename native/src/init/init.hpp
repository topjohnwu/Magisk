#pragma once

#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"
#define REDIR_PATH "/data/magiskinit"

#define PRELOAD_LIB    "/dev/preload.so"
#define PRELOAD_POLICY "/dev/sepolicy"
#define PRELOAD_ACK    "/dev/ack"

#ifdef __cplusplus

#include <base.hpp>
#include <stream.hpp>

#include "init-rs.hpp"

int magisk_proxy_main(int, char *argv[]);
rust::Utf8CStr backup_init();

#endif
