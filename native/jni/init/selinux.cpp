#include <sys/mount.h>

#include <magisk.hpp>
#include <sepolicy.hpp>
#include <utils.hpp>

#include "init.hpp"

using namespace std;

void MagiskInit::patch_sepolicy(const char *file) {
    LOGD("Patching monolithic policy\n");
    auto sepol = unique_ptr<sepolicy>(sepolicy::from_file("/sepolicy"));

    sepol->magisk_rules();

    // Custom rules
    if (!custom_rules_dir.empty()) {
        if (auto dir = xopen_dir(custom_rules_dir.data())) {
            for (dirent *entry; (entry = xreaddir(dir.get()));) {
                auto rule = custom_rules_dir + "/" + entry->d_name + "/sepolicy.rule";
                if (xaccess(rule.data(), R_OK) == 0) {
                    LOGD("Loading custom sepolicy patch: [%s]\n", rule.data());
                    sepol->load_rule_file(rule.data());
                }
            }
        }
    }

    LOGD("Dumping sepolicy to: [%s]\n", file);
    sepol->to_file(file);

    // Remove OnePlus stupid debug sepolicy and use our own
    if (access("/sepolicy_debug", F_OK) == 0) {
        unlink("/sepolicy_debug");
        link("/sepolicy", "/sepolicy_debug");
    }
}

#define MOCK_POLICY    SELINUXMOCK "/sepolicy"
#define MOCK_PLAT_VER  SELINUXMOCK "/plat_sepolicy_vers.txt"
#define MOCK_NULL      SELINUXMOCK "/null"

bool MagiskInit::hijack_sepolicy() {
    xmkdir(SELINUXMOCK, 0);

    std::string vend_plat_vers;
    file_readline(SPLIT_PLAT_VER, [&vend_plat_vers] (auto line){
        vend_plat_vers = line;
        return false;
    });

    if (vend_plat_vers.empty()) return false;

    // SPLIT_PLAT_VER will be open read only right after creating policy tmp file
    // Mock it to block init, get policy tmp file and mock it
    LOGD("Hijack [%s]\n", SPLIT_PLAT_VER);
    mkfifo(MOCK_PLAT_VER, 0644);
    xmount(MOCK_PLAT_VER, SPLIT_PLAT_VER, nullptr, MS_BIND, nullptr);

    // Read all custom rules into memory
    string rules;
    if (!custom_rules_dir.empty()) {
        if (auto dir = xopen_dir(custom_rules_dir.data())) {
            for (dirent *entry; (entry = xreaddir(dir.get()));) {
                auto rule_file = custom_rules_dir + "/" + entry->d_name + "/sepolicy.rule";
                if (xaccess(rule_file.data(), R_OK) == 0) {
                    LOGD("Load custom sepolicy patch: [%s]\n", rule_file.data());
                    full_read(rule_file.data(), rules);
                    rules += '\n';
                }
            }
        }
    }

    // Create a new process waiting for init operations
    if (xfork()) {
        // In parent, return and continue boot process
        return true;
    }

    // block until SPLIT_PLAT_VER opened as read only
    // we have selinuxfs now and init has opened a tmp policy file
    int fd = xopen(MOCK_PLAT_VER, O_WRONLY);
    umount2("/init", MNT_DETACH);
    xumount2(SPLIT_PLAT_VER, MNT_DETACH);

    // fnd the tmp policy file by searching init's open files
    std::string policy_path;
    {
        auto dir = open_dir("/proc/1/fd");
        for (dirent *entry; (entry = xreaddir(dir.get()));) {
            char buf[PATH_MAX];
            if (readlinkat(dirfd(dir.get()), entry->d_name, buf, sizeof(buf)) < 0) continue;
            if (auto path = std::string_view(buf); path.starts_with("/dev/sepolicy.")) {
                policy_path = path;
            }
        }
    }

    // hijack policy file to let secilc write the compiled policy for us
    LOGD("Hijack [%s]\n", policy_path.data());
    mkfifo(MOCK_POLICY, 0644);
    xmount(MOCK_POLICY, policy_path.data(), nullptr, MS_BIND, nullptr);

    // hijack null file to block secilc until we finish our sepolicy patching and write to the
    // real tmp policy file
    LOGD("Hijack [%s]\n", SELINUX_NULL);
    mkfifo(MOCK_NULL, 0644);
    xmount(MOCK_NULL, SELINUX_NULL, nullptr, MS_BIND, nullptr);

    write(fd, vend_plat_vers.data(), vend_plat_vers.size());
    {
        // block until secilc open as write only
        fd = xopen(MOCK_POLICY, O_RDONLY);
        xumount2(policy_path.data(), MNT_DETACH);

        std::string policy;
        // read the compiled policy
        fd_full_read(fd, policy);

        // path the compiled policy
        auto sepol = unique_ptr<sepolicy>(sepolicy::from_data(policy.data(), policy.length()));
        sepol->magisk_rules();
        sepol->load_rules(rules);
        // save to the original tmp policy file
        sepol->to_file(policy_path.data());
    }

    {
        std::string dummy;
        // block seclic until we finish patching policy
        fd = xopen(MOCK_NULL, O_RDONLY);
        xumount2(SELINUX_NULL, MNT_DETACH);
        // read all dummy content
        fd_full_read(fd, dummy);
    }

    // gracefully exit
    exit(0);
}
