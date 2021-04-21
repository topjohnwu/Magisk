#include <sys/mount.h>
#include <map>
#include <utility>

#include <utils.hpp>
#include <magisk.hpp>
#include <selinux.hpp>
#include <resetprop.hpp>

#include "core.hpp"

using namespace std;

#define VLOGD(tag, from, to) LOGD("%-8s: %s <- %s\n", tag, to, from)

#define TYPE_MIRROR  (1 << 0)    /* mount from mirror */
#define TYPE_INTER   (1 << 1)    /* intermediate node */
#define TYPE_SKEL    (1 << 2)    /* replace with tmpfs */
#define TYPE_MODULE  (1 << 3)    /* mount from module */
#define TYPE_ROOT    (1 << 4)    /* partition root */
#define TYPE_CUSTOM  (1 << 5)    /* custom node type overrides all */
#define TYPE_DIR     (TYPE_INTER|TYPE_SKEL|TYPE_ROOT)

static vector<string> module_list;

class node_entry;
class dir_node;
class inter_node;
class mirror_node;
class skel_node;
class module_node;
class root_node;

static void merge_node(node_entry *a, node_entry *b);
template<class T> static bool isa(node_entry *node);
static int bind_mount(const char *from, const char *to) {
    int ret = xmount(from, to, nullptr, MS_BIND, nullptr);
    if (ret == 0)
        VLOGD("bind_mnt", from, to);
    return ret;
}

template<class T> uint8_t type_id() { return TYPE_CUSTOM; }
template<> uint8_t type_id<dir_node>() { return TYPE_DIR; }
template<> uint8_t type_id<inter_node>() { return TYPE_INTER; }
template<> uint8_t type_id<mirror_node>() { return TYPE_MIRROR; }
template<> uint8_t type_id<skel_node>() { return TYPE_SKEL; }
template<> uint8_t type_id<module_node>() { return TYPE_MODULE; }
template<> uint8_t type_id<root_node>() { return TYPE_ROOT; }

class node_entry {
public:
    virtual ~node_entry() = default;

    bool is_dir() { return file_type() == DT_DIR; }
    bool is_lnk() { return file_type() == DT_LNK; }
    bool is_reg() { return file_type() == DT_REG; }

    uint8_t type() { return node_type; }
    const string &name() { return _name; }

    // Paths
    const string &node_path();
    string mirror_path() { return mirror_dir + node_path(); }

    dir_node *parent() { return _parent; }

    virtual void mount() = 0;

    static string module_mnt;
    static string mirror_dir;

protected:
    template<class T>
    node_entry(const char *name, uint8_t file_type, T*)
    : _name(name), _file_type(file_type), node_type(type_id<T>()) {}

    template<class T>
    explicit node_entry(T*) : node_type(type_id<T>()) {}

    void create_and_mount(const string &src);

    /* Use top bit of _file_type for node exist status */
    bool exist() { return static_cast<bool>(_file_type & (1 << 7)); }
    void set_exist(bool b) { if (b) _file_type |= (1 << 7); else _file_type &= ~(1 << 7); }
    uint8_t file_type() { return static_cast<uint8_t>(_file_type & ~(1 << 7)); }

private:
    friend void merge_node(node_entry *a, node_entry *b);
    friend class dir_node;

    bool need_skel_upgrade(node_entry *child);

    // Node properties
    string _name;
    uint8_t _file_type;
    uint8_t node_type;

    dir_node *_parent = nullptr;

    // Cache
    string _node_path;
};

class dir_node : public node_entry {
public:
    friend void merge_node(node_entry *a, node_entry *b);

    typedef map<string_view, node_entry *> map_type;
    typedef map_type::iterator map_iter;

    ~dir_node() override {
        for (auto &it : children)
            delete it.second;
        children.clear();
    }

    // Return false to indicate need to upgrade to module
    bool collect_files(const char *module, int dfd);

    // Return false to indicate need to upgrade to skeleton
    bool prepare();

    // Default directory mount logic
    void mount() override {
        for (auto &pair : children)
            pair.second->mount();
    }

    /***************
     * Tree Methods
     ***************/

    bool is_empty() { return children.empty(); }

    template<class T>
    T *child(string_view name) { return iter_to_node<T>(children.find(name)); }

    // Lazy val
    root_node *root() {
        if (!_root)
            _root = _parent->root();
        return _root;
    }

    // Return child with name or nullptr
    node_entry *extract(string_view name);

    // Return false if rejected
    bool insert(node_entry *node) {
        return node
        ? iter_to_node(insert(children.find(node->_name), node->node_type,
                [=](auto _) { return node; })) != nullptr
        : false;
    }

    // Return inserted node or null if rejected
    template<class T, class ...Args>
    T *emplace(string_view name, Args &&...args) {
        return iter_to_node<T>(insert(children.find(name), type_id<T>(),
                [&](auto _) { return new T(std::forward<Args>(args)...); }));
    }

    // Return inserted node, existing node with same rank, or null
    template<class T, class ...Args>
    T *emplace_or_get(string_view name, Args &&...args) {
        return iter_to_node<T>(insert(children.find(name), type_id<T>(),
                [&](auto _) { return new T(std::forward<Args>(args)...); }, true));
    }

    // Return upgraded node or null if rejected
    template<class T, class ...Args>
    T *upgrade(string_view name, Args &...args) {
        return iter_to_node<T>(upgrade<T>(children.find(name), args...));
    }

protected:
    template<class T>
    dir_node(const char *name, uint8_t file_type, T *self) : node_entry(name, file_type, self) {
        if constexpr (std::is_same_v<T, root_node>)
            _root = self;
    }

    template<class T>
    dir_node(node_entry *node, T *self) : node_entry(self) {
        merge_node(this, node);
        if constexpr (std::is_same_v<T, root_node>)
            _root = self;
    }

    template<class T>
    dir_node(const char *name, T *self) : dir_node(name, DT_DIR, self) {}

    template<class T = node_entry>
    T *iter_to_node(const map_iter &it) {
        return reinterpret_cast<T*>(it == children.end() ? nullptr : it->second);
    }

    /* fn signature: (node_ent *&) -> node_ent *
     * fn gets reference to the existing node, may be null.
     * If fn consumes input, need to set reference to null.
     * Returns new node or null to reject the insertion. */

    template<typename Func>
    map_iter insert(map_iter it, uint8_t type, Func fn, bool allow_same = false);

    template<class To, class From = node_entry, class ...Args>
    map_iter upgrade(map_iter it, Args &&...args) {
        return insert(it, type_id<To>(), [&](node_entry *&ex) -> node_entry * {
            if (!ex)
                return nullptr;
            if constexpr (!std::is_same_v<From, node_entry>) {
                // Type check if specific type is selected
                if (!isa<From>(ex))
                    return nullptr;
            }
            auto node = new To(reinterpret_cast<From *>(ex), std::forward<Args>(args)...);
            ex = nullptr;
            return node;
        });
    }

    // dir nodes host children
    map_type children;

    // Root node lookup cache
    root_node *_root = nullptr;
};

class root_node : public dir_node {
public:
    explicit root_node(const char *name) : dir_node(name, this), prefix("") {}
    explicit root_node(node_entry *node) : dir_node(node, this), prefix("/system") {}
    const char * const prefix;
};

class inter_node : public dir_node {
public:
    inter_node(const char *name, const char *module) : dir_node(name, this), module(module) {}
private:
    const char *module;
    friend class module_node;
};

class module_node : public node_entry {
public:
    module_node(const char *module, dirent *entry)
    : node_entry(entry->d_name, entry->d_type, this), module(module) {}

    module_node(node_entry *node, const char *module) : node_entry(this), module(module) {
        merge_node(this, node);
    }

    explicit module_node(inter_node *node) : module_node(node, node->module) {}

    void mount() override;
private:
    const char *module;
};

class mirror_node : public node_entry {
public:
    explicit mirror_node(dirent *entry) : node_entry(entry->d_name, entry->d_type, this) {}
    void mount() override {
        create_and_mount(mirror_path());
    }
};

class skel_node : public dir_node {
public:
    explicit skel_node(node_entry *node);
    void mount() override;
};

// Poor man's dynamic cast without RTTI
template<class T>
static bool isa(node_entry *node) {
    return node && (node->type() & type_id<T>());
}
template<class T>
static T *dyn_cast(node_entry *node) {
    return isa<T>(node) ? reinterpret_cast<T*>(node) : nullptr;
}

// Merge b -> a, b will be deleted
static void merge_node(node_entry *a, node_entry *b) {
    a->_name.swap(b->_name);
    a->_file_type = b->_file_type;
    a->_parent = b->_parent;

    // Merge children if both is dir
    if (auto aa = dyn_cast<dir_node>(a); aa) {
        if (auto bb = dyn_cast<dir_node>(b); bb) {
            aa->children.merge(bb->children);
            for (auto &pair : aa->children)
                pair.second->_parent = aa;
        }
    }
    delete b;
}

string node_entry::module_mnt;
string node_entry::mirror_dir;

const string &node_entry::node_path() {
    if (_parent && _node_path.empty())
        _node_path = _parent->node_path() + '/' + _name;
    return _node_path;
}

/*************************
 * Node Tree Construction
 *************************/

template<typename Func>
dir_node::map_iter dir_node::insert(dir_node::map_iter it, uint8_t type, Func fn, bool allow_same) {
    node_entry *node = nullptr;
    if (it != children.end()) {
        if (it->second->node_type < type) {
            // Upgrade existing node only if higher precedence
            node = fn(it->second);
            if (!node)
                return children.end();
            if (it->second)
                merge_node(node, it->second);
            it = children.erase(it);
            // Minor optimization to make insert O(1) by using hint
            if (it == children.begin())
                it = children.emplace(node->_name, node).first;
            else
                it = children.emplace_hint(--it, node->_name, node);
        } else {
            if (allow_same && it->second->node_type == type)
                return it;
            return children.end();
        }
    } else {
        node = fn(node);
        if (!node)
            return it;
        node->_parent = this;
        it = children.emplace(node->_name, node).first;
    }
    return it;
}

node_entry* dir_node::extract(string_view name) {
    auto it = children.find(name);
    if (it != children.end()) {
        auto ret = it->second;
        children.erase(it);
        return ret;
    }
    return nullptr;
}

skel_node::skel_node(node_entry *node) : dir_node(node, this) {
    string mirror = mirror_path();
    if (auto dir = open_dir(mirror.data()); dir) {
        set_exist(true);
        for (dirent *entry; (entry = xreaddir(dir.get()));) {
            // Insert mirror nodes
            emplace<mirror_node>(entry->d_name, entry);
        }
    } else {
        // It is actually possible that mirror does not exist (nested mount points)
        // Set self to non exist so this node will be ignored at mount
        set_exist(false);
        return;
    }

    for (auto it = children.begin(); it != children.end(); ++it) {
        // Need to upgrade all inter_node children to skel_node
        if (isa<inter_node>(it->second))
            it = upgrade<skel_node>(it);
    }
}

bool node_entry::need_skel_upgrade(node_entry *child) {
    /* We need to upgrade to skeleton if:
     * - Target does not exist
     * - Source or target is a symlink */
    bool upgrade = false;
    struct stat st;
    if (lstat(child->node_path().data(), &st) != 0) {
        upgrade = true;
    } else {
        child->set_exist(true);
        if (child->is_lnk() || S_ISLNK(st.st_mode))
            upgrade = true;
    }
    return upgrade;
}

bool dir_node::prepare() {
    bool to_skel = false;
    for (auto it = children.begin(); it != children.end();) {
        if (need_skel_upgrade(it->second)) {
            if (node_type > type_id<skel_node>()) {
                // Upgrade will fail, remove the unsupported child node
                delete it->second;
                it = children.erase(it);
                continue;
            }
            // Tell parent to upgrade self to skel
            to_skel = true;
            // If child is inter_node, upgrade to module
            if (auto nit = upgrade<module_node, inter_node>(it); nit != children.end()) {
                it = nit;
                goto next_node;
            }
        }
        if (auto dn = dyn_cast<dir_node>(it->second); dn && dn->is_dir() && !dn->prepare()) {
            // Upgrade child to skeleton
            it = upgrade<skel_node>(it);
        }
next_node:
        ++it;
    }
    return !to_skel;
}

bool dir_node::collect_files(const char *module, int dfd) {
    auto dir = xopen_dir(xopenat(dfd, _name.data(), O_RDONLY | O_CLOEXEC));
    if (!dir)
        return true;

    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        if (entry->d_name == ".replace"sv) {
            // Stop traversing and tell parent to upgrade self to module
            return false;
        }

        if (entry->d_type == DT_DIR) {
            // Need check cause emplace could fail due to previous module dir replace
            if (auto dn = emplace_or_get<inter_node>(entry->d_name, entry->d_name, module);
                dn && !dn->collect_files(module, dirfd(dir.get()))) {
                // Upgrade node to module due to '.replace'
                upgrade<module_node>(dn->name(), module);
            }
        } else {
            emplace<module_node>(entry->d_name, module, entry);
        }
    }
    return true;
}

/************************
 * Mount Implementations
 ************************/

void node_entry::create_and_mount(const string &src) {
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
        bind_mount(src.data(), dest.data());
    }
}

void module_node::mount() {
    string src = module_mnt + module + parent()->root()->prefix + node_path();
    if (exist())
        clone_attr(mirror_path().data(), src.data());
    if (isa<skel_node>(parent()))
        create_and_mount(src);
    else if (is_dir() || is_reg())
        bind_mount(src.data(), node_path().data());
}

void skel_node::mount() {
    if (!exist())
        return;
    string src = mirror_path();
    const string &dest = node_path();
    file_attr a;
    getattr(src.data(), &a);
    mkdir(dest.data(), 0);
    if (!isa<skel_node>(parent())) {
        // We don't need another layer of tmpfs if parent is skel
        xmount("tmpfs", dest.data(), "tmpfs", 0, nullptr);
        VLOGD("mnt_tmp", "tmpfs", dest.data());
    }
    setattr(dest.data(), &a);
    dir_node::mount();
}

/****************
 * Magisk Stuffs
 ****************/

class magisk_node : public node_entry {
public:
    explicit magisk_node(const char *name) : node_entry(name, DT_REG, this) {}

    void mount() override {
        const string &dir_name = parent()->node_path();
        if (name() == "magisk") {
            for (int i = 0; applet_names[i]; ++i) {
                string dest = dir_name + "/" + applet_names[i];
                VLOGD("create", "./magisk", dest.data());
                xsymlink("./magisk", dest.data());
            }
        } else {
            for (int i = 0; init_applet[i]; ++i) {
                string dest = dir_name + "/" + init_applet[i];
                VLOGD("create", "./magiskinit", dest.data());
                xsymlink("./magiskinit", dest.data());
            }
        }
        create_and_mount(MAGISKTMP + "/" + name());
    }
};

static void inject_magisk_bins(root_node *system) {
    auto bin = system->child<inter_node>("bin");
    if (!bin) {
        bin = new inter_node("bin", "");
        system->insert(bin);
    }

    // Insert binaries
    bin->insert(new magisk_node("magisk"));
    bin->insert(new magisk_node("magiskinit"));

    // Also delete all applets to make sure no modules can override it
    for (int i = 0; applet_names[i]; ++i)
        delete bin->extract(applet_names[i]);
    for (int i = 0; init_applet[i]; ++i)
        delete bin->extract(init_applet[i]);
}

void magic_mount() {
    node_entry::mirror_dir = MAGISKTMP + "/" MIRRDIR;
    node_entry::module_mnt = MAGISKTMP + "/" MODULEMNT "/";

    auto root = make_unique<root_node>("");
    auto system = new root_node("system");
    root->insert(system);

    char buf[4096];
    LOGI("* Loading modules\n");
    for (const auto &m : module_list) {
        auto module = m.data();
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
        system->collect_files(module, fd);
        close(fd);
    }

    if (MAGISKTMP != "/sbin") {
        // Need to inject our binaries into /system/bin
        inject_magisk_bins(system);
    }

    if (system->is_empty())
        return;

    // Handle special read-only partitions
    for (const char *part : { "/vendor", "/product", "/system_ext" }) {
        struct stat st;
        if (lstat(part, &st) == 0 && S_ISDIR(st.st_mode)) {
            if (auto old = system->extract(part + 1); old) {
                auto new_node = new root_node(old);
                root->insert(new_node);
            }
        }
    }

    root->prepare();
    root->mount();
}

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

    // Setup module mount (workaround nosuid selabel issue)
    auto src = MAGISKTMP + "/" MIRRDIR MODULEROOT;
    auto dest = MAGISKTMP + "/" MODULEMNT;
    xmkdir(dest.data(), 0755);
    bind_mount(src.data(), dest.data());

    restorecon();
    chmod(SECURE_DIR, 0700);
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

static void collect_modules() {
    foreach_module([](int dfd, dirent *entry, int modfd) {
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
        if (faccessat(modfd, "disable", F_OK, 0) != 0)
            module_list.emplace_back(entry->d_name);
    });
}

void handle_modules() {
    prepare_modules();
    collect_modules();
    exec_module_scripts("post-fs-data");

    // Recollect modules (module scripts could remove itself)
    module_list.clear();
    collect_modules();
}

void disable_modules() {
    foreach_module([](int, auto, int modfd) {
        close(xopenat(modfd, "disable", O_RDONLY | O_CREAT | O_CLOEXEC, 0));
    });
}

void remove_modules() {
    foreach_module([](int, dirent *entry, int) {
        auto uninstaller = MODULEROOT + "/"s + entry->d_name + "/uninstall.sh";
        if (access(uninstaller.data(), F_OK) == 0)
            exec_script(uninstaller.data());
    });
    rm_rf(MODULEROOT);
}

void exec_module_scripts(const char *stage) {
    exec_module_scripts(stage, module_list);
}
