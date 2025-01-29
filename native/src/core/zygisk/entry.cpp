#include <libgen.h>
#include <dlfcn.h>
#include <sys/prctl.h>
#include <sys/mount.h>
#include <android/log.h>
#include <android/dlext.h>

#include <base.hpp>
#include <consts.hpp>

#include "zygisk.hpp"
#include "module.hpp"

using namespace std;

string native_bridge = "0";

static bool is_compatible_with(uint32_t) {
    zygisk_logging();
    hook_entry();
    ZLOGD("load success\n");
    return false;
}

extern "C" [[maybe_unused]] NativeBridgeCallbacks NativeBridgeItf {
    .version = 2,
    .padding = {},
    .isCompatibleWith = &is_compatible_with,
};

// The following code runs in magiskd

void restore_zygisk_prop() {
    string native_bridge_orig = "0";
    if (native_bridge.length() > strlen(ZYGISKLDR)) {
        native_bridge_orig = native_bridge.substr(strlen(ZYGISKLDR));
    }
    set_prop(NBPROP, native_bridge_orig.data());
}
