#include <base.hpp>
#include <stream.hpp>

#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"
#define INIT_PATH  "/system/bin/init"
#define REDIR_PATH "/data/magiskinit"

int magisk_proxy_main(int argc, char *argv[]);

#include "init-rs.hpp"
