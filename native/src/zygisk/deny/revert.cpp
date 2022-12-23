#include <set>
#include <sys/mount.h>
#include <sys/sysmacros.h>

#include <magisk.hpp>
#include <base.hpp>

#include "deny.hpp"

using namespace std;

static void lazy_unmount(const char* mountpoint) {
    if (umount2(mountpoint, MNT_DETACH) != -1)
        LOGD("denylist: Unmounted (%s)\n", mountpoint);
}

void revert_daemon(int pid, int client) {
    if (fork_dont_care() == 0) {
        revert_unmount(pid);
        write_int(client, DenyResponse::OK);
        _exit(0);
    }
}

void revert_unmount(int pid, const char *ref_pid) {
    vector<string> targets;
    set<unsigned> peer_groups;
    {
        auto mount_info = ParseMountInfo(ref_pid);
        {
            // in case someone mounts something from magisktmp to a private mount points globally
            auto global_mount_info = ParseMountInfo("1");
            mount_info.insert(mount_info.end(),
                              std::make_move_iterator(global_mount_info.begin()),
                              std::make_move_iterator(global_mount_info.end()));
        }
        if (pid > 0) {
            if (switch_mnt_ns(pid))
                return;
            LOGD("denylist: handling PID=[%d]\n", pid);
        }
        {
            // in case someone mounts something from magisktmp to a private mount points privately
            auto self_mount_info = ParseMountInfo("self");
            mount_info.insert(mount_info.end(),
                              std::make_move_iterator(self_mount_info.begin()),
                              std::make_move_iterator(self_mount_info.end()));
        }
        for (auto &info : mount_info) {
            if (info.target.starts_with(MAGISKTMP) || peer_groups.contains(info.optional.master) ||
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
