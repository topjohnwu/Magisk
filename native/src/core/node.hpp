#pragma once

#include <sys/mount.h>
#include <map>

using namespace std;

#define TYPE_INTER   (1 << 0)    /* intermediate node */
#define TYPE_TMPFS   (1 << 1)    /* replace with tmpfs */
#define TYPE_MODULE  (1 << 2)    /* mount from module */
#define TYPE_ROOT    (1 << 3)    /* partition root */
#define TYPE_CUSTOM  (1 << 4)    /* custom node type overrides all */
#define TYPE_DIR     (TYPE_INTER|TYPE_TMPFS|TYPE_ROOT)

class node_entry;
class dir_node;
class inter_node;
class tmpfs_node;
class module_node;
class root_node;

// Poor man's dynamic cast without RTTI
template<class T> static bool isa(node_entry *node);
template<class T> static T *dyn_cast(node_entry *node);
template<class T> uint8_t type_id() { return TYPE_CUSTOM; }
template<> uint8_t type_id<dir_node>() { return TYPE_DIR; }
template<> uint8_t type_id<inter_node>() { return TYPE_INTER; }
template<> uint8_t type_id<tmpfs_node>() { return TYPE_TMPFS; }
template<> uint8_t type_id<module_node>() { return TYPE_MODULE; }
template<> uint8_t type_id<root_node>() { return TYPE_ROOT; }

class node_entry {
public:
    virtual ~node_entry() = default;

    // Node info
    bool is_dir() const { return file_type() == DT_DIR; }
    bool is_lnk() const { return file_type() == DT_LNK; }
    bool is_reg() const { return file_type() == DT_REG; }
    bool is_wht() const { return file_type() == DT_WHT; }
    const string &name() const { return _name; }
    dir_node *parent() const { return _parent; }

    // Don't call the following two functions before prepare
    const string &node_path();
    const string worker_path();

    virtual void mount() = 0;

    inline static string module_mnt;

protected:
    template<class T>
    node_entry(const char *name, uint8_t file_type, T*)
    : _name(name), _file_type(file_type & 15), _node_type(type_id<T>()) {}

    template<class T>
    explicit node_entry(T*) : _file_type(0), _node_type(type_id<T>()) {}

    virtual void consume(node_entry *other) {
        _name.swap(other->_name);
        _file_type = other->_file_type;
        _parent = other->_parent;
        delete other;
    }

    void create_and_mount(const char *reason, const string &src, bool ro=false);

    // Use bit 7 of _file_type for exist status
    bool exist() const { return static_cast<bool>(_file_type & (1 << 7)); }
    void set_exist(bool b) { if (b) _file_type |= (1 << 7); else _file_type &= ~(1 << 7); }

private:
    friend class dir_node;

    template<class T>
    friend bool isa(node_entry *node);

    uint8_t file_type() const { return static_cast<uint8_t>(_file_type & 15); }

    // Node properties
    string _name;
    dir_node *_parent = nullptr;

    // Cache, it should only be used within prepare
    string _node_path;

    uint8_t _file_type;
    const uint8_t _node_type;
};

class dir_node : public node_entry {
public:
    using map_type = map<string_view, node_entry *>;
    using iterator = map_type::iterator;

    ~dir_node() override {
        for (auto &it : children)
            delete it.second;
        children.clear();
    }

    /**************
     * Entrypoints
     **************/

    // Traverse through module directories to generate a tree of module files
    void collect_module_files(const char *module, int dfd);

    // Traverse through the real filesystem and prepare the tree for magic mount.
    // Return true to indicate that this node needs to be upgraded to tmpfs_node.
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
    T *get_child(string_view name) { return iterator_to_node<T>(children.find(name)); }

    root_node *root() {
        if (!_root)
            _root = _parent->root();
        return _root;
    }

    // Return child with name or nullptr
    node_entry *extract(string_view name) {
        auto it = children.find(name);
        if (it != children.end()) {
            auto ret = it->second;
            children.erase(it);
            return ret;
        }
        return nullptr;
    }

    // Return false if rejected
    bool insert(node_entry *node) {
        auto fn = [=](auto) { return node; };
        return node && iterator_to_node(insert(node->_name, node->_node_type, fn));
    }

    // Return inserted node or null if rejected
    template<class T, class ...Args>
    T *emplace(string_view name, Args &&...args) {
        auto fn = [&](auto) { return new T(std::forward<Args>(args)...); };
        return iterator_to_node<T>(insert(name, type_id<T>(), fn));
    }

    // Return upgraded node or null if rejected
    template<class T, class ...Args>
    T *upgrade(string_view name, Args &...args) {
        return iterator_to_node<T>(upgrade<T>(children.find(name), args...));
    }

protected:
    template<class T>
    dir_node(const char *name, T *self) : node_entry(name, DT_DIR, self) {
        if constexpr (std::is_same_v<T, root_node>)
            _root = self;
    }

    template<class T>
    dir_node(dirent *entry, T *self) : node_entry(entry->d_name, entry->d_type, self) {
        if constexpr (std::is_same_v<T, root_node>)
            _root = self;
    }

    template<class T>
    dir_node(node_entry *node, T *self) : node_entry(self) {
        if constexpr (std::is_same_v<T, root_node>)
            _root = self;
        dir_node::consume(node);
    }

    void consume(node_entry *other) override {
        if (auto o = dyn_cast<dir_node>(other)) {
            children.merge(o->children);
            for (auto &pair : children)
                pair.second->_parent = this;
        }
        node_entry::consume(other);
    }

    // Use bit 6 of _file_type
    // Skip binding mirror for this directory
    bool replace() const { return static_cast<bool>(_file_type & (1 << 6)); }
    void set_replace(bool b) { if (b) _file_type |= (1 << 6); else _file_type &= ~(1 << 6); }

    template<class T = node_entry>
    T *iterator_to_node(iterator it) {
        return static_cast<T*>(it == children.end() ? nullptr : it->second);
    }

    template<typename Builder>
    iterator insert(string_view name, uint8_t type, const Builder &builder) {
        return insert_at(children.find(name), type, builder);
    }

    // Emplace insert a new node, or upgrade if the requested type has a higher rank.
    // Return iterator to the new/upgraded node, or end() if insertion is rejected.
    // fn is the node builder function. Signature: (node_entry *&) -> node_entry *
    // fn gets a reference to the existing node pointer and returns a new node object.
    // Input is null when there is no existing node. If returns null, the insertion is rejected.
    // If fn consumes the input, it should set the reference to null.
    template<typename Builder>
    iterator insert_at(iterator it, uint8_t type, const Builder &builder) {
        node_entry *node = nullptr;
        if (it != children.end()) {
            // Upgrade existing node only if higher rank
            if (it->second->_node_type < type) {
                node = builder(it->second);
                if (!node)
                    return children.end();
                if (it->second)
                    node->consume(it->second);
                it = children.erase(it);
                // Minor optimization to make insert O(1) by using hint
                if (it == children.begin())
                    it = children.emplace(node->_name, node).first;
                else
                    it = children.emplace_hint(--it, node->_name, node);
            } else {
                return children.end();
            }
        } else {
            node = builder(node);
            if (!node)
                return children.end();
            node->_parent = this;
            it = children.emplace(node->_name, node).first;
        }
        return it;
    }

    template<class T, class ...Args>
    iterator upgrade(iterator it, Args &&...args) {
        return insert_at(it, type_id<T>(), [&](node_entry *&ex) -> node_entry * {
            if (!ex) return nullptr;
            auto node = new T(ex, std::forward<Args>(args)...);
            ex = nullptr;
            return node;
        });
    }

    // dir nodes host children
    map_type children;

private:
    // Root node lookup cache
    root_node *_root = nullptr;
};

class root_node : public dir_node {
public:
    explicit root_node(const char *name) : dir_node(name, this), prefix("") {
        set_exist(true);
    }
    explicit root_node(node_entry *node) : dir_node(node, this), prefix("/system") {
        set_exist(true);
    }
    const char * const prefix;
};

class inter_node : public dir_node {
public:
    inter_node(const char *name) : dir_node(name, this) {}
    inter_node(dirent *entry) : dir_node(entry, this) {}
};

class module_node : public node_entry {
public:
    module_node(const char *module, dirent *entry)
    : node_entry(entry->d_name, entry->d_type, this), module(module) {}

    module_node(node_entry *node, const char *module) : node_entry(this), module(module) {
        node_entry::consume(node);
    }

    void mount() override;
private:
    const char *module;
};

// Don't create tmpfs_node before prepare
class tmpfs_node : public dir_node {
public:
    explicit tmpfs_node(node_entry *node);
    void mount() override;
};

template<class T>
static bool isa(node_entry *node) {
    return node && (node->_node_type & type_id<T>());
}
template<class T>
static T *dyn_cast(node_entry *node) {
    return isa<T>(node) ? static_cast<T*>(node) : nullptr;
}

const string &node_entry::node_path() {
    if (_parent && _node_path.empty())
        _node_path = _parent->node_path() + '/' + _name;
    return _node_path;
}

const string node_entry::worker_path() {
    return get_magisk_tmp() + "/"s WORKERDIR + node_path();
}
