#include <sys/mount.h>
#include <map>
#include <utility>

#include <base.hpp>
#include <magisk.hpp>
#include <daemon.hpp>
#include <selinux.hpp>
#include <resetprop.hpp>

#include "core.hpp"
#include "node.hpp"
#include "embed.hpp"

using namespace std;

#define VLOGD(tag, from, to) LOGD("%-8s: %s <- %s\n", tag, to, from)

namespace {
int bind_mount(const char *reason, const char *from, const char *to) {
    int ret = xmount(from, to, nullptr, MS_BIND | MS_REC, nullptr);
    if (ret == 0)
        VLOGD(reason, from, to);
    return ret;
}

std::atomic_uint zygote_start_count{1};

template<bool is64bit>
std::string prepare_zygisk_loader() {
    static std::string loader = [] {
        auto zygisk_bin = MAGISKTMP + "/" ZYGISKBIN;
        string src = zygisk_bin + "/libzygisk_loader" + (is64bit ? "64" : "32") + ".so";
        xmkdir(zygisk_bin.data(), 0);
        int out = xopen(src.data(), O_WRONLY | O_CREAT | O_CLOEXEC, 0);
        if constexpr (is64bit) {
#ifdef __LP64__
            xwrite(out, zygisk64_ld, sizeof(zygisk64_ld));
#endif
        } else {
            xwrite(out, zygisk32_ld, sizeof(zygisk32_ld));
        }
        close(out);
        return src;
    }();
    return loader;
}

template<bool is64bit>
void mount_zygisk(bool rollback) {
    auto lib = "/system/lib"s + (is64bit ? "64" : "") + "/" + native_bridge;
    auto loader = prepare_zygisk_loader<is64bit>();
    if (struct stat lib_st{}, loader_st{}; // use to check if mount is needed
            xstat(lib.data(), &lib_st) == 0 && xstat(loader.data(), &loader_st) == 0) {
        if (!rollback && (lib_st.st_ino != loader_st.st_ino || lib_st.st_dev != loader_st.st_dev)) {
            // Our native bridge hasn't been mounted yet or has been unmounted before
            xmount(loader.data(), lib.data(), nullptr, MS_BIND, nullptr);
        } else if (rollback && lib_st.st_ino == loader_st.st_ino && lib_st.st_dev == loader_st.st_dev) {
            // We have mounted our native bridge overrode the original one, unmount it for rollback
            xumount2(lib.data(), MNT_DETACH);
        }
    }
}

}

string node_entry::module_mnt;
string node_entry::mirror_dir;

/*************************
 * Node Tree Construction
 *************************/

tmpfs_node::tmpfs_node(node_entry *node) : dir_node(node, this) {
    if (!skip_mirror()) {
        string mirror = mirror_path();
        if (auto dir = open_dir(mirror.data())) {
            set_exist(true);
            for (dirent *entry; (entry = xreaddir(dir.get()));) {
                if (entry->d_type == DT_DIR) {
                    // create a dummy inter_node to upgrade later
                    emplace<inter_node>(entry->d_name, entry->d_name);
                } else {
                    // Insert mirror nodes
                    emplace<mirror_node>(entry->d_name, entry);
                }
            }
        }
    }

    for (auto it = children.begin(); it != children.end(); ++it) {
        // Need to upgrade all inter_node children to tmpfs_node
        if (isa<inter_node>(it->second))
            it = upgrade<tmpfs_node>(it);
    }
}

bool dir_node::prepare() {
    // If direct replace or not exist, mount ourselves as tmpfs
    bool upgrade_to_tmpfs = skip_mirror() || !exist();

    for (auto it = children.begin(); it != children.end();) {
        // We also need to upgrade to tmpfs node if any child:
        // - Target does not exist
        // - Source or target is a symlink (since we cannot bind mount symlink)
        bool cannot_mnt;
        if (struct stat st{}; lstat(it->second->node_path().data(), &st) != 0) {
            cannot_mnt = true;
        } else {
            it->second->set_exist(true);
            cannot_mnt = it->second->is_lnk() || S_ISLNK(st.st_mode);
        }

        if (cannot_mnt) {
            if (_node_type > type_id<tmpfs_node>()) {
                // Upgrade will fail, remove the unsupported child node
                LOGW("Unable to add: %s, skipped\n", it->second->node_path().data());
                delete it->second;
                it = children.erase(it);
                continue;
            }
            upgrade_to_tmpfs = true;
        }
        if (auto dn = dyn_cast<dir_node>(it->second)) {
            if (skip_mirror()) {
                // Propagate skip mirror state to all children
                dn->set_skip_mirror(true);
            }
            if (dn->prepare()) {
                // Upgrade child to tmpfs
                it = upgrade<tmpfs_node>(it);
            }
        }
        ++it;
    }
    return upgrade_to_tmpfs;
}

void dir_node::collect_module_files(const char *module, int dfd) {
    auto dir = xopen_dir(xopenat(dfd, name().data(), O_RDONLY | O_CLOEXEC));
    if (!dir)
        return;

    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        if (entry->d_name == ".replace"sv) {
            set_skip_mirror(true);
            continue;
        }

        if (entry->d_type == DT_DIR) {
            inter_node *node;
            if (auto it = children.find(entry->d_name); it == children.end()) {
                node = emplace<inter_node>(entry->d_name, entry->d_name);
            } else {
                node = dyn_cast<inter_node>(it->second);
            }
            if (node) {
                node->collect_module_files(module, dirfd(dir.get()));
            }
        } else {
            emplace<module_node>(entry->d_name, module, entry);
        }
    }
}

/************************
 * Mount Implementations
 ************************/

void node_entry::create_and_mount(const char *reason, const string &src) {
    const string &dest = node_path();
    if (is_lnk()) {
        VLOGD("cp_link", src.data(), dest.data());
        cp_afc(src.data(), dest.data());
    } else {
        if (is_dir())
            xmkdir(dest.data(), 0);
        else if (is_reg())
            close(xopen(dest.data(), O_RDONLY | O_CREAT | O_CLOEXEC, 0));
        else
            return;
        bind_mount(reason, src.data(), dest.data());
    }
}

void mirror_node::mount() {
    create_and_mount("mirror", mirror_path());
}

void module_node::mount() {
    string src = module_mnt + module + parent()->root()->prefix + node_path();
    if (exist())
        clone_attr(mirror_path().data(), src.data());
    if (isa<tmpfs_node>(parent()))
        create_and_mount("module", src);
    else
        bind_mount("module", src.data(), node_path().data());
}

void tmpfs_node::mount() {
    string src = mirror_path();
    const string &dest = node_path();
    file_attr a{};
    if (access(src.data(), F_OK) == 0)
        getattr(src.data(), &a);
    else
        getattr(parent()->node_path().data(), &a);
    if (!isa<tmpfs_node>(parent())) {
        auto worker_dir = MAGISKTMP + "/" WORKERDIR + dest;
        mkdirs(worker_dir.data(), 0);
        create_and_mount(skip_mirror() ? "replace" : "tmpfs", worker_dir);
    } else {
        // We don't need another layer of tmpfs if parent is tmpfs
        mkdir(dest.data(), 0);
    }
    setattr(dest.data(), &a);
    dir_node::mount();
}

/****************
 * Magisk Stuffs
 ****************/

void magisk_node::mount() {
    const string src = MAGISKTMP + "/" + name();
    if (access(src.data(), F_OK))
        return;

    const string &dir_name = parent()->node_path();
    if (name() == "magisk") {
        for (int i = 0; applet_names[i]; ++i) {
            string dest = dir_name + "/" + applet_names[i];
            VLOGD("create", "./magisk", dest.data());
            xsymlink("./magisk", dest.data());
        }
    } else if (name() == "magiskpolicy") {
        string dest = dir_name + "/supolicy";
        VLOGD("create", "./magiskpolicy", dest.data());
        xsymlink("./magiskpolicy", dest.data());
    }
    create_and_mount("magisk", src);
}

template<bool is64bit>
void zygisk_node<is64bit>::mount() {
    auto src = prepare_zygisk_loader<is64bit>();
    create_and_mount("zygisk", src);
}

static void inject_magisk_bins(root_node *system) {
    auto bin = system->get_child<inter_node>("bin");
    if (!bin) {
        bin = new inter_node("bin");
        system->insert(bin);
    }

    // Insert binaries
    bin->insert(new magisk_node("magisk"));
    if (access("/system/bin/linker32", F_OK) == 0)
        bin->insert(new magisk_node("magisk32"));
    if (access("/system/bin/linker64", F_OK) == 0)
        bin->insert(new magisk_node("magisk64"));
    bin->insert(new magisk_node("magiskpolicy"));

    // Also delete all applets to make sure no modules can override it
    for (int i = 0; applet_names[i]; ++i)
        delete bin->extract(applet_names[i]);
    delete bin->extract("supolicy");
}

vector<module_info> *module_list;

void load_modules() {
    node_entry::mirror_dir = MAGISKTMP + "/" MIRRDIR;
    node_entry::module_mnt = MAGISKTMP + "/" MODULEMNT "/";

    auto root = make_unique<root_node>("");
    auto system = new root_node("system");
    root->insert(system);

    char buf[4096];
    LOGI("* Loading modules\n");
    for (const auto &m : *module_list) {
        const char *module = m.name.data();
        char *b = buf + sprintf(buf, "%s/" MODULEMNT "/%s/", MAGISKTMP.data(), module);

        // Read props
        strcpy(b, "system.prop");
        if (access(buf, F_OK) == 0) {
            LOGI("%s: loading [system.prop]\n", module);
            load_prop_file(buf, false);
        }

        // Check whether skip mounting
        strcpy(b, "skip_mount");
        if (access(buf, F_OK) == 0)
            continue;

        // Double check whether the system folder exists
        strcpy(b, "system");
        if (access(buf, F_OK) != 0)
            continue;

        LOGI("%s: loading mount files\n", module);
        b[-1] = '\0';
        int fd = xopen(buf, O_RDONLY | O_CLOEXEC);
        system->collect_module_files(module, fd);
        close(fd);
    }

    inject_magisk_bins(system);

    if (zygisk_enabled) {
        native_bridge = getprop(NBPROP);
        // If the system has built-in native bridge, we mount on top of it to avoid unnecessary property changes
        // If not, we change the property to install our fake native bridge
        if (native_bridge.empty() || native_bridge == "0") {
            native_bridge = ZYGISKLDR;
            setprop(NBPROP, ZYGISKLDR, false);
            if (access("/system/lib", F_OK) == 0) {
                auto lib = system->get_child<inter_node>("lib");
                if (!lib) {
                    lib = new inter_node("lib");
                    system->insert(lib);
                }
                lib->insert(new zygisk_node<false>(native_bridge.data()));
            }

            if (access("/system/lib64", F_OK) == 0) {
                auto lib64 = system->get_child<inter_node>("lib64");
                if (!lib64) {
                    lib64 = new inter_node("lib64");
                    system->insert(lib64);
                }
                lib64->insert(new zygisk_node<true>(native_bridge.data()));
            }
        }

        // Weather Huawei's Maple compiler is enabled.
        // If so, system server will be created by a special Zygote which ignores the native bridge
        // and make system server out of our control. Avoid it by disabling.
        if (getprop("ro.maple.enable") == "1") {
            setprop("ro.maple.enable", "0", false);
        }
    }

    if (!system->is_empty()) {
        // Handle special read-only partitions
        for (const char *part : { "/vendor", "/product", "/system_ext" }) {
            struct stat st{};
            if (lstat(part, &st) == 0 && S_ISDIR(st.st_mode)) {
                if (auto old = system->extract(part + 1)) {
                    auto new_node = new root_node(old);
                    root->insert(new_node);
                }
            }
        }
        root->prepare();
        root->mount();
    }
    if (native_bridge != ZYGISKLDR) {
        mount_zygisk<true>(false);
        mount_zygisk<false>(false);
    }
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

static void collect_modules(bool open_zygisk) {
    foreach_module([=](int dfd, dirent *entry, int modfd) {
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

        module_info info;
        if (zygisk_enabled) {
            // Riru and its modules are not compatible with zygisk
            if (entry->d_name == "riru-core"sv || faccessat(modfd, "riru", F_OK, 0) == 0) {
                LOGI("%s: ignore\n", entry->d_name);
                return;
            }
            if (open_zygisk) {
#if defined(__arm__)
                info.z32 = openat(modfd, "zygisk/armeabi-v7a.so", O_RDONLY | O_CLOEXEC);
#elif defined(__aarch64__)
                info.z32 = openat(modfd, "zygisk/armeabi-v7a.so", O_RDONLY | O_CLOEXEC);
                info.z64 = openat(modfd, "zygisk/arm64-v8a.so", O_RDONLY | O_CLOEXEC);
#elif defined(__i386__)
                info.z32 = openat(modfd, "zygisk/x86.so", O_RDONLY | O_CLOEXEC);
#elif defined(__x86_64__)
                info.z32 = openat(modfd, "zygisk/x86.so", O_RDONLY | O_CLOEXEC);
                info.z64 = openat(modfd, "zygisk/x86_64.so", O_RDONLY | O_CLOEXEC);
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
        module_list->push_back(info);
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
        std::for_each(module_list->begin(), module_list->end(), [&](module_info &info) {
            info.z32 = convert_to_memfd(info.z32);
#if defined(__LP64__)
            info.z64 = convert_to_memfd(info.z64);
#endif
        });
    }
}

void handle_modules() {
    prepare_modules();
    collect_modules(false);
    exec_module_scripts("post-fs-data");

    // Recollect modules (module scripts could remove itself)
    module_list->clear();
    collect_modules(true);
}

static int check_rules_dir(char *buf, size_t sz) {
    int off = ssprintf(buf, sz, "%s/%s", MAGISKTMP.data(), PREINITMIRR);
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

void exec_module_scripts(const char *stage) {
    vector<string_view> module_names;
    std::transform(module_list->begin(), module_list->end(), std::back_inserter(module_names),
        [](const module_info &info) -> string_view { return info.name; });
    exec_module_scripts(stage, module_names);
}

// Used to remount zygisk after soft reboot
void remount_zygisk() {
    bool rollback = false;
    if (zygote_start_count.fetch_add(1) >= 5) {
        LOGW("zygote crashes too many times, rolling-back\n");
        rollback = true;
    }
    if (native_bridge == ZYGISKLDR) {
        rollback ? setprop(NBPROP, "0") : setprop(NBPROP, ZYGISKLDR);
    } else {
        mount_zygisk<false>(rollback);
        mount_zygisk<true>(rollback);
    }
}
