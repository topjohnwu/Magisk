#include <base.hpp>
#include <stream.hpp>

#include "init-rs.hpp"

#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"
#define REDIR_PATH "/data/magiskinit"

int magisk_proxy_main(int, char *argv[]);
rust::Utf8CStr backup_init();
