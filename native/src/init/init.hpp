#include <base.hpp>
#include <stream.hpp>

#include "init-rs.hpp"

#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"
#define INIT_PATH  "/system/bin/init"
#define REDIR_PATH "/data/magiskinit"

int magisk_proxy_main(int argc, char *argv[]);
bool unxz(out_stream &strm, rust::Slice<const uint8_t> bytes);
bool check_two_stage();
const char *backup_init();
void restore_ramdisk_init();
