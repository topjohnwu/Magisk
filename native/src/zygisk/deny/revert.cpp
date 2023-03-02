#include <sys/mount.h>

#include <magisk.hpp>
#include <base.hpp>

#include "deny.hpp"

using namespace std;

static void lazy_unmount(const char* mountpoint) {
    if (umount2(mountpoint, MNT_DETACH) != -1)
        LOGD("denylist: Unmounted (%s)\n", mountpoint);
}

void revert_unmount() {
    vector<string> targets;

    // Unmount dummy skeletons and MAGISKTMP
    // since mirror nodes are always mounted under skeleton, we don't have to specifically unmount
    for (auto &info: parse_mount_info("self")) {
        if (info.source == "magisk" || info.source == "worker" || // magisktmp tmpfs
            info.root.starts_with("/adb/modules")) { // bind mount from data partition
            targets.push_back(info.target);
        }
    }

    for (auto &s : targets)
        lazy_unmount(s.data());
}
