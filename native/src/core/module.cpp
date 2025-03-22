#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <map>
#include <utility>

#include <base.hpp>
#include <consts.hpp>
#include <core.hpp>
#include <selinux.hpp>

using namespace std;

static void load_modules(bool zygisk_enabled, const rust::Vec<ModuleInfo> &module_list) {
    char buf[4096];
    LOGI("* Loading modules\n");
    for (const auto &m : module_list) {
        char *b = buf + ssprintf(buf, sizeof(buf), "%s/" MODULEMNT "/%.*s/",
                                 get_magisk_tmp(), (int) m.name.size(), m.name.data());

        // Read props
        strcpy(b, "system.prop");
        if (access(buf, F_OK) == 0) {
            LOGI("%.*s: loading [system.prop]\n", (int) m.name.size(), m.name.data());
            // Do NOT go through property service as it could cause boot lock
            load_prop_file(buf, true);
        }
    }
    std::string magisk_path;
    if (get_magisk_tmp() != "/sbin"sv || !str_contains(getenv("PATH") ?: "", "/sbin")) {
        // Need to inject our binaries into /system/bin
        magisk_path = "/system/bin";
        for (struct stat st{}; auto &item: split(getenv("PATH"), ":")) {
            item.erase(0, item.starts_with("/system/") ? 8 : 1);
            auto &&system_path = "/system/"s + item;
            if (stat(system_path.data(), &st) == 0 && st.st_mode & S_IXOTH) {
                magisk_path = std::move(system_path);
                break;
            }
        }
    }

    if (zygisk_enabled) {
        string native_bridge_orig = get_prop(NBPROP);
        if (native_bridge_orig.empty()) {
            native_bridge_orig = "0";
        }
        native_bridge = native_bridge_orig != "0" ? ZYGISKLDR + native_bridge_orig : ZYGISKLDR;
        set_prop(NBPROP, native_bridge.data());
        // Weather Huawei's Maple compiler is enabled.
        // If so, system server will be created by a special Zygote which ignores the native bridge
        // and make system server out of our control. Avoid it by disabling.
        if (get_prop("ro.maple.enable") == "1") {
            set_prop("ro.maple.enable", "0");
        }
    }

    deploy_modules(module_list, native_bridge, magisk_path);
}

/************************
 * Filesystem operations
 ************************/

static void prepare_modules() {
    // Upgrade modules
    if (auto dir = open_dir(MODULEUPGRADE); dir) {
        int ufd = dirfd(dir.get());
        int mfd = xopen(MODULEROOT, O_RDONLY | O_CLOEXEC);
        for (dirent *entry; (entry = xreaddir(dir.get()));) {
            if (entry->d_type == DT_DIR) {
                // Cleanup old module if exists
                if (faccessat(mfd, entry->d_name, F_OK, 0) == 0) {
                    int modfd = xopenat(mfd, entry->d_name, O_RDONLY | O_CLOEXEC);
                    if (faccessat(modfd, "disable", F_OK, 0) == 0) {
                        auto disable = entry->d_name + "/disable"s;
                        close(xopenat(ufd, disable.data(), O_RDONLY | O_CREAT | O_CLOEXEC, 0));
                    }
                    frm_rf(modfd);
                    unlinkat(mfd, entry->d_name, AT_REMOVEDIR);
                }
                LOGI("Upgrade / New module: %s\n", entry->d_name);
                renameat(ufd, entry->d_name, mfd, entry->d_name);
            }
        }
        close(mfd);
        rm_rf(MODULEUPGRADE);
    }
}

template<typename Func>
static void foreach_module(Func fn) {
    auto dir = open_dir(MODULEROOT);
    if (!dir)
        return;

    int dfd = dirfd(dir.get());
    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        if (entry->d_type == DT_DIR && entry->d_name != ".core"sv) {
            int modfd = xopenat(dfd, entry->d_name, O_RDONLY | O_CLOEXEC);
            fn(dfd, entry, modfd);
            close(modfd);
        }
    }
}

static rust::Vec<ModuleInfo> collect_modules(bool zygisk_enabled, bool open_zygisk) {
    rust::Vec<ModuleInfo> modules;
    foreach_module([&](int dfd, dirent *entry, int modfd) {
        if (faccessat(modfd, "remove", F_OK, 0) == 0) {
            LOGI("%s: remove\n", entry->d_name);
            auto uninstaller = MODULEROOT + "/"s + entry->d_name + "/uninstall.sh";
            if (access(uninstaller.data(), F_OK) == 0)
                exec_script(uninstaller.data());
            frm_rf(xdup(modfd));
            unlinkat(dfd, entry->d_name, AT_REMOVEDIR);
            return;
        }
        unlinkat(modfd, "update", 0);
        if (faccessat(modfd, "disable", F_OK, 0) == 0)
            return;

        ModuleInfo info{{}, -1, -1};
        if (zygisk_enabled) {
            // Riru and its modules are not compatible with zygisk
            if (entry->d_name == "riru-core"sv || faccessat(modfd, "riru", F_OK, 0) == 0) {
                LOGI("%s: ignore\n", entry->d_name);
                return;
            }
            if (open_zygisk) {
#if defined(__arm__)
                info.z32 = openat(modfd, "zygisk/armeabi-v7a.so", O_RDONLY | O_CLOEXEC);
                info.z64 = -1;
#elif defined(__aarch64__)
                info.z32 = openat(modfd, "zygisk/armeabi-v7a.so", O_RDONLY | O_CLOEXEC);
                info.z64 = openat(modfd, "zygisk/arm64-v8a.so", O_RDONLY | O_CLOEXEC);
#elif defined(__i386__)
                info.z32 = openat(modfd, "zygisk/x86.so", O_RDONLY | O_CLOEXEC);
                info.z64 = -1;
#elif defined(__x86_64__)
                info.z32 = openat(modfd, "zygisk/x86.so", O_RDONLY | O_CLOEXEC);
                info.z64 = openat(modfd, "zygisk/x86_64.so", O_RDONLY | O_CLOEXEC);
#elif defined(__riscv)
                info.z32 = -1;
                info.z64 = openat(modfd, "zygisk/riscv64.so", O_RDONLY | O_CLOEXEC);
#else
#error Unsupported ABI
#endif
                unlinkat(modfd, "zygisk/unloaded", 0);
            }
        } else {
            // Ignore zygisk modules when zygisk is not enabled
            if (faccessat(modfd, "zygisk", F_OK, 0) == 0) {
                LOGI("%s: ignore\n", entry->d_name);
                return;
            }
        }
        info.name = entry->d_name;
        modules.push_back(std::move(info));
    });
    if (zygisk_enabled) {
        bool use_memfd = true;
        auto convert_to_memfd = [&](int fd) -> int {
            if (fd < 0)
                return -1;
            if (use_memfd) {
                int memfd = syscall(__NR_memfd_create, "jit-cache", MFD_CLOEXEC);
                if (memfd >= 0) {
                    xsendfile(memfd, fd, nullptr, INT_MAX);
                    close(fd);
                    return memfd;
                } else {
                    // memfd_create failed, just use what we had
                    use_memfd = false;
                }
            }
            return fd;
        };
        std::for_each(modules.begin(),modules.end(), [&](ModuleInfo &info) {
            info.z32 = convert_to_memfd(info.z32);
            info.z64 = convert_to_memfd(info.z64);
        });
    }
    return modules;
}

rust::Vec<ModuleInfo> MagiskD::handle_modules() const noexcept {
    bool zygisk = zygisk_enabled();
    prepare_modules();
    exec_module_scripts("post-fs-data", collect_modules(zygisk, false));
    // Recollect modules (module scripts could remove itself)
    auto list = collect_modules(zygisk, true);
    load_modules(zygisk, list);
    return list;
}

static int check_rules_dir(char *buf, size_t sz) {
    int off = ssprintf(buf, sz, "%s/" PREINITMIRR, get_magisk_tmp());
    struct stat st1{};
    struct stat st2{};
    if (xstat(buf, &st1) < 0 || xstat(MODULEROOT, &st2) < 0)
        return 0;
    if (st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino)
        return 0;
    return off;
}

void disable_modules() {
    char buf[4096];
    int off = check_rules_dir(buf, sizeof(buf));
    foreach_module([&](int, dirent *entry, int modfd) {
        close(xopenat(modfd, "disable", O_RDONLY | O_CREAT | O_CLOEXEC, 0));
        if (off) {
            ssprintf(buf + off, sizeof(buf) - off, "/%s/sepolicy.rule", entry->d_name);
            unlink(buf);
        }
    });
}

void remove_modules() {
    char buf[4096];
    int off = check_rules_dir(buf, sizeof(buf));
    foreach_module([&](int, dirent *entry, int) {
        auto uninstaller = MODULEROOT + "/"s + entry->d_name + "/uninstall.sh";
        if (access(uninstaller.data(), F_OK) == 0)
            exec_script(uninstaller.data());
        if (off) {
            ssprintf(buf + off, sizeof(buf) - off, "/%s/sepolicy.rule", entry->d_name);
            unlink(buf);
        }
    });
    rm_rf(MODULEROOT);
}
