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
#define MOCK_BLOCKING  SELINUXMOCK "/blocking"
#define MOCK_NULL      SELINUXMOCK "/null"

inline void hijack(const char* src, const char* dest) {
    LOGD("Hijack [%s]\n", dest);
    mkfifo(src, 0644);
    xmount(src, dest, nullptr, MS_BIND, nullptr);
}

bool MagiskInit::hijack_sepolicy() {
    xmkdir(SELINUXMOCK, 0);

    std::string actual_content;
    const char *blocking_file;
    bool pre_compiled = false;
    if (sepolicy::check_precompiled(ODM_PRE_COMPILED)) {
        blocking_file = ODM_PRE_COMPILED;
        pre_compiled = true;
    } else if (sepolicy::check_precompiled(VEND_PRE_COMPILED)) {
        blocking_file = VEND_PRE_COMPILED;
        pre_compiled = true;
    } else {
        blocking_file = SPLIT_PLAT_VER;
        file_readline(SPLIT_PLAT_VER, [&actual_content](auto line) {
            actual_content = line;
            return false;
        });
        if (actual_content.empty()) return false;
    }
    // this will be open read only right after creating policy tmp file if not pre-compiled
    // Mock it to block init, get policy tmp file and mock it or patch the pre-compiled policy
    hijack(MOCK_BLOCKING, blocking_file);

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

    // Block until blocking file opened as read only
    // selinuxfs has been mounted now
    int fd = xopen(MOCK_BLOCKING, O_WRONLY);
    umount2("/init", MNT_DETACH);
    xumount2(blocking_file, MNT_DETACH);

    std::string policy_path;
    if (pre_compiled) {
        policy_path = MOCK_BLOCKING;
    } else {
        // init has opened a tmp policy file
        // Find it by searching init's open files
        {
            auto dir = open_dir("/proc/1/fd");
            for (dirent *entry; (entry = xreaddir(dir.get()));) {
                char buf[PATH_MAX];
                if (readlinkat(dirfd(dir.get()), entry->d_name, buf, sizeof(buf)) < 0) continue;
                if (auto path = std::string_view(buf); path.starts_with("/dev/sepolicy.")) {
                    policy_path = path;
                    break;
                }
            }
        }

        // Hijack the tmp file to let secilc write the compiled policy to us
        hijack(MOCK_POLICY, policy_path.data());

        // Hijack null file to block secilc until we finish our sepolicy patching and write to the
        // original tmp policy file
        hijack(MOCK_NULL, SELINUX_NULL);

        write(fd, actual_content.data(), actual_content.size());
        close(fd);
        {
            // Block until secilc opens it as write only
            fd = xopen(MOCK_POLICY, O_RDONLY);
            xumount2(policy_path.data(), MNT_DETACH);

            // Read the compiled policy
            fd_full_read(fd, actual_content);
            close(fd);
        }
    }
    // Patch the compiled policy
    auto sepol = std::unique_ptr<sepolicy>(
            sepolicy::from_data(actual_content.data(), actual_content.length()));
    sepol->magisk_rules();
    sepol->load_rules(rules);
    // Save to the right destination
    sepol->to_file(policy_path.data());
    if (!pre_compiled) {
        // Unlock seclic when we finish patching policy
        fd = xopen(MOCK_NULL, O_RDONLY);
        xumount2(SELINUX_NULL, MNT_DETACH);
        // Read all dummy content
        fd_full_read(fd, actual_content);
    }
    close(fd);
    // All done, exit gracefully
    exit(0);
}
