#include <sys/mount.h>
#include <mntinfo.hpp>
#include <sys/stat.h>

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
    struct stat data_stat;
    int data_stat_ret = stat("/data", &data_stat);
    set<unsigned> peer_groups;
    {
        auto mount_info = ParseMountInfo("1");
        {
            auto self_mount_info = ParseMountInfo("self");
            mount_info.insert(mount_info.end(),
                              std::make_move_iterator(self_mount_info.begin()),
                              std::make_move_iterator(self_mount_info.end()));
        }
        for (auto &info : mount_info) {
            if (data_stat_ret == 0 && info.device == data_stat.st_dev && info.root.starts_with("/adb/modules/")){
                targets.emplace_back(std::move(info.target));
                continue;
            }
            if (info.target.starts_with(MAGISKTMP) || info.root.starts_with("/" INTLROOT "/") || info.source.starts_with(MAGISKTMP) ||
                (info.source == "magisktmpfs" && info.type == "tmpfs") ||
                peer_groups.contains(info.optional.master) ||
                peer_groups.contains(info.optional.shared)) {
                targets.emplace_back(std::move(info.target));
                if (info.optional.master != 0) {
                    peer_groups.insert(info.optional.master);
                }
                if (info.optional.shared != 0) {
                    peer_groups.insert(info.optional.shared);
                }
            }
        }
    }
    for (auto &s : targets)
        lazy_unmount(s.data());
}
