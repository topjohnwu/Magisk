#include <base.hpp>

#include "cpio.hpp"
#include "magiskboot.hpp"
#include "compress.hpp"

using namespace std;

static const char *UNSUPPORT_LIST[] =
        { "sbin/launch_daemonsu.sh", "sbin/su", "init.xposed.rc",
          "boot/sbin/launch_daemonsu.sh" };

static const char *MAGISK_LIST[] =
        { ".backup/.magisk", "init.magisk.rc",
          "overlay/init.magisk.rc" };

class magisk_cpio : public cpio {
public:
    void patch();
    int test();
    char *sha1();
    void restore();
    void backup(const char *orig);
};

bool check_env(const char *name) {
    const char *val = getenv(name);
    return val != nullptr && val == "true"sv;
}

void magisk_cpio::patch() {
    bool keepverity = check_env("KEEPVERITY");
    bool keepforceencrypt = check_env("KEEPFORCEENCRYPT");
    fprintf(stderr, "Patch with flag KEEPVERITY=[%s] KEEPFORCEENCRYPT=[%s]\n",
            keepverity ? "true" : "false", keepforceencrypt ? "true" : "false");

    for (auto it = entries.begin(); it != entries.end();) {
        auto cur = it++;
        bool fstab = (!keepverity || !keepforceencrypt) &&
                     S_ISREG(cur->second->mode) &&
                     !str_starts(cur->first, ".backup") &&
                     !str_contains(cur->first, "twrp") &&
                     !str_contains(cur->first, "recovery") &&
                     str_contains(cur->first, "fstab");
        if (!keepverity) {
            if (fstab) {
                fprintf(stderr, "Found fstab file [%s]\n", cur->first.data());
                cur->second->filesize = patch_verity(cur->second->data, cur->second->filesize);
            } else if (cur->first == "verity_key") {
                rm(cur);
                continue;
            }
        }
        if (!keepforceencrypt) {
            if (fstab) {
                cur->second->filesize = patch_encryption(cur->second->data, cur->second->filesize);
            }
        }
    }
}

#define MAGISK_PATCHED    (1 << 0)
#define UNSUPPORTED_CPIO  (1 << 1)
#define SONY_INIT         (1 << 2)

int magisk_cpio::test() {
    int ret = 0;
    for (auto file : UNSUPPORT_LIST) {
        if (exists(file)) {
            return UNSUPPORTED_CPIO;
        }
    }
    for (auto file : MAGISK_LIST) {
        if (exists(file)) {
            ret |= MAGISK_PATCHED;
            break;
        }
    }
    if (exists("init.real"))
        ret |= SONY_INIT;
    return ret;
}

#define for_each_line(line, buf, size) \
for (char *line = (char *) buf; line < (char *) buf + size && line[0]; line = strchr(line + 1, '\n') + 1)

char *magisk_cpio::sha1() {
    char sha1[41];
    for (auto &e : entries) {
        if (e.first == "init.magisk.rc" || e.first == "overlay/init.magisk.rc") {
            for_each_line(line, e.second->data, e.second->filesize) {
                if (strncmp(line, "#STOCKSHA1=", 11) == 0) {
                    strncpy(sha1, line + 12, 40);
                    sha1[40] = '\0';
                    return strdup(sha1);
                }
            }
        } else if (e.first == ".backup/.magisk") {
            for_each_line(line, e.second->data, e.second->filesize) {
                if (str_starts(line, "SHA1=")) {
                    strncpy(sha1, line + 5, 40);
                    sha1[40] = '\0';
                    return strdup(sha1);
                }
            }
        } else if (e.first == ".backup/.sha1") {
            return (char *) e.second->data;
        }
    }
    return nullptr;
}

#define for_each_str(str, buf, size) \
for (char *str = (char *) buf; str < (char *) buf + size; str += strlen(str) + 1)

void magisk_cpio::restore() {
    // Collect files
    auto bk = entries.end();
    auto rl = entries.end();
    auto mg = entries.end();
    vector<entry_map::iterator> backups;
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (it->first == ".backup") {
            bk = it;
        } else if (it->first == ".backup/.rmlist") {
            rl = it;
        } else if (it->first == ".backup/.magisk") {
            mg = it;
        } else if (str_starts(it->first, ".backup/")) {
            backups.emplace_back(it);
        }
    }

    // If the .backup folder is effectively empty, this means that the boot ramdisk was
    // created from scratch by an old broken magiskboot. This is just a hacky workaround.
    if (bk != entries.end() && mg != entries.end() && rl == entries.end() && backups.empty()) {
        fprintf(stderr, "Remove all in ramdisk\n");
        entries.clear();
        return;
    }

    // Remove files
    rm(bk);
    rm(mg);
    if (rl != entries.end()) {
        for_each_str(file, rl->second->data, rl->second->filesize) {
            rm(file);
        }
        rm(rl);
    }

    // Restore files
    for (auto it : backups) {
        const char *name = &it->first[8];
        mv(it, name);
    }
}

void magisk_cpio::backup(const char *orig) {
    entry_map backups;
    string rm_list;
    backups.emplace(".backup", new cpio_entry(S_IFDIR));

    magisk_cpio o;
    if (access(orig, R_OK) == 0)
        o.load_cpio(orig);

    // Remove existing backups in original ramdisk
    o.rm(".backup", true);
    rm(".backup", true);

    auto lhs = o.entries.begin();
    auto rhs = entries.begin();

    while (lhs != o.entries.end() || rhs != entries.end()) {
        int res;
        bool do_backup = false;
        if (lhs != o.entries.end() && rhs != entries.end()) {
            res = lhs->first.compare(rhs->first);
        } else if (lhs == o.entries.end()) {
            res = 1;
        } else {
            res = -1;
        }

        if (res < 0) {
            // Something is missing in new ramdisk, do_backup!
            do_backup = true;
            fprintf(stderr, "Backup missing entry: ");
        } else if (res == 0) {
            if (lhs->second->filesize != rhs->second->filesize ||
                memcmp(lhs->second->data, rhs->second->data, lhs->second->filesize) != 0) {
                // Not the same!
                do_backup = true;
                fprintf(stderr, "Backup mismatch entry: ");
            }
        } else {
            // Something new in ramdisk
            rm_list += rhs->first;
            rm_list += (char) '\0';
            fprintf(stderr, "Record new entry: [%s] -> [.backup/.rmlist]\n", rhs->first.data());
        }

        if (do_backup) {
            string name = ".backup/" + lhs->first;
            fprintf(stderr, "[%s] -> [%s]\n", lhs->first.data(), name.data());
            auto e = lhs->second.release();
            backups.emplace(name, e);
        }

        // Increment positions
        if (res < 0) {
            ++lhs;
        } else if (res == 0) {
            ++lhs; ++rhs;
        } else {
            ++rhs;
        }
    }

    if (!rm_list.empty()) {
        auto rm_list_file = new cpio_entry(S_IFREG);
        rm_list_file->filesize = rm_list.length();
        rm_list_file->data = malloc(rm_list.length());
        memcpy(rm_list_file->data, rm_list.data(), rm_list.length());
        backups.emplace(".backup/.rmlist", rm_list_file);
    }

    if (backups.size() > 1)
        entries.merge(backups);
}

int cpio_commands(int argc, char *argv[]) {
    char *incpio = argv[0];
    ++argv;
    --argc;

    magisk_cpio cpio;
    if (access(incpio, R_OK) == 0)
        cpio.load_cpio(incpio);

    int cmdc;
    char *cmdv[6];

    for (int i = 0; i < argc; ++i) {
        // Reset
        cmdc = 0;
        memset(cmdv, 0, sizeof(cmdv));

        // Split the commands
        char *tok = strtok(argv[i], " ");
        while (tok && cmdc < std::size(cmdv)) {
            if (cmdc == 0 && tok[0] == '#')
                break;
            cmdv[cmdc++] = tok;
            tok = strtok(nullptr, " ");
        }

        if (cmdc == 0)
            continue;

        if (cmdv[0] == "test"sv) {
            exit(cpio.test());
        } else if (cmdv[0] == "restore"sv) {
            cpio.restore();
        } else if (cmdv[0] == "sha1"sv) {
            char *sha1 = cpio.sha1();
            if (sha1) printf("%s\n", sha1);
            return 0;
        } else if (cmdv[0] == "patch"sv) {
            cpio.patch();
        } else if (cmdc == 2 && cmdv[0] == "exists"sv) {
            exit(!cpio.exists(cmdv[1]));
        } else if (cmdc == 2 && cmdv[0] == "backup"sv) {
            cpio.backup(cmdv[1]);
        } else if (cmdc >= 2 && cmdv[0] == "rm"sv) {
            bool r = cmdc > 2 && cmdv[1] == "-r"sv;
            cpio.rm(cmdv[1 + r], r);
        } else if (cmdc == 3 && cmdv[0] == "mv"sv) {
            cpio.mv(cmdv[1], cmdv[2]);
        } else if (cmdv[0] == "extract"sv) {
            if (cmdc == 3) {
                return !cpio.extract(cmdv[1], cmdv[2]);
            } else {
                cpio.extract();
                return 0;
            }
        } else if (cmdc == 3 && cmdv[0] == "mkdir"sv) {
            cpio.mkdir(strtoul(cmdv[1], nullptr, 8), cmdv[2]);
        } else if (cmdc == 3 && cmdv[0] == "ln"sv) {
            cpio.ln(cmdv[1], cmdv[2]);
        } else if (cmdc == 4 && cmdv[0] == "add"sv) {
            cpio.add(strtoul(cmdv[1], nullptr, 8), cmdv[2], cmdv[3]);
        } else {
            return 1;
        }
    }

    cpio.dump(incpio);
    return 0;
}
