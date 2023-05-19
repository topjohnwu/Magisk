#include <dlfcn.h>
#include <sys/types.h>
#include <vector>
#include <map>

#include <resetprop.hpp>
#include <base.hpp>

#include "prop.hpp"

using namespace std;

#ifdef APPLET_STUB_MAIN
#define system_property_set             __system_property_set
#define system_property_find            __system_property_find
#define system_property_read_callback   __system_property_read_callback
#define system_property_foreach         __system_property_foreach
#define system_property_read(...)
#else
static int (*system_property_set)(const char*, const char*);
static int (*system_property_read)(const prop_info*, char*, char*);
static const prop_info *(*system_property_find)(const char*);
static void (*system_property_read_callback)(
        const prop_info*, void (*)(void*, const char*, const char*, uint32_t), void*);
static int (*system_property_foreach)(void (*)(const prop_info*, void*), void*);
#endif

struct PropFlags {
    void setSkipSvc() { flags |= 1; }
    void setPersist() { flags |= (1 << 1); }
    void setContext() { flags |= (1 << 2); }
    void setDelete() { flags |= (1 << 3); }
    void setLoadFile() { flags |= (1 << 4); }
    bool isSkipSvc() const { return flags & 1; }
    bool isPersist() const { return flags & (1 << 1); }
    bool isContext() const { return flags & (1 << 2); }
    bool isDelete() const { return flags & (1 << 3); }
    bool isLoadFile() const { return flags & (1 << 4); }
private:
    uint32_t flags = 0;
};

[[noreturn]] static void usage(char* arg0) {
    fprintf(stderr,
R"EOF(resetprop - System Property Manipulation Tool

Usage: %s [flags] [options...]

Options:
   -h, --help        show this message
   (no arguments)    print all properties
   NAME              get property
   NAME VALUE        set property entry NAME with VALUE
   --file FILE       load props from FILE
   --delete NAME     delete property

Flags:
   -v      print verbose output to stderr
   -n      set props without going through property_service
           (this flag only affects setprop)
   -p      read/write props from/to persistent storage
           (this flag only affects getprop and delprop)
   -Z      show property contexts instead of values
           (this flag only affects getprop)

)EOF", arg0);
    exit(1);
}

static bool check_legal_property_name(const char *name) {
    int namelen = strlen(name);

    if (namelen < 1) goto illegal;
    if (name[0] == '.') goto illegal;
    if (name[namelen - 1] == '.') goto illegal;

    /* Only allow alphanumeric, plus '.', '-', '@', ':', or '_' */
    /* Don't allow ".." to appear in a property name */
    for (size_t i = 0; i < namelen; i++) {
        if (name[i] == '.') {
            // i=0 is guaranteed to never have a dot. See above.
            if (name[i-1] == '.') goto illegal;
            continue;
        }
        if (name[i] == '_' || name[i] == '-' || name[i] == '@' || name[i] == ':') continue;
        if (name[i] >= 'a' && name[i] <= 'z') continue;
        if (name[i] >= 'A' && name[i] <= 'Z') continue;
        if (name[i] >= '0' && name[i] <= '9') continue;
        goto illegal;
    }

    return true;

illegal:
    LOGE("Illegal property name: [%s]\n", name);
    return false;
}

static void read_prop_with_cb(const prop_info *pi, void *cb) {
    if (system_property_read_callback) {
        auto callback = [](void *cb, const char *name, const char *value, uint32_t) {
            static_cast<prop_cb*>(cb)->exec(name, value);
        };
        system_property_read_callback(pi, callback, cb);
    } else {
        char name[PROP_NAME_MAX];
        char value[PROP_VALUE_MAX];
        name[0] = '\0';
        value[0] = '\0';
        system_property_read(pi, name, value);
        static_cast<prop_cb*>(cb)->exec(name, value);
    }
}

struct prop_to_string : prop_cb {
    explicit prop_to_string(string &s) : val(s) {}
    void exec(const char *, const char *value) override {
        val = value;
    }
private:
    string &val;
};

struct resetprop {

    resetprop() {

#ifndef APPLET_STUB_MAIN
#define DLOAD(name) (*(void **) &name = dlsym(RTLD_DEFAULT, "__" #name))
        // Load platform implementations
        DLOAD(system_property_set);
        DLOAD(system_property_read);
        DLOAD(system_property_find);
        DLOAD(system_property_read_callback);
        DLOAD(system_property_foreach);
#undef DLOAD
#endif

        if (__system_properties_init()) {
            LOGE("resetprop: __system_properties_init error\n");
        }
    }


    int set_prop(const char *name, const char *value, PropFlags flags) {
        if (!check_legal_property_name(name))
            return 1;

        const char *msg = flags.isSkipSvc() ? "modifying prop data structure" : "property_service";

        auto pi = const_cast<prop_info *>(__system_property_find(name));

        // Always delete existing read-only properties, because they could be
        // long properties and cannot directly go through __system_property_update
        if (pi != nullptr && str_starts(name, "ro.")) {
            // Skip pruning nodes as we will add it back ASAP
            __system_property_delete(name, false);
            pi = nullptr;
        }

        int ret;
        if (pi != nullptr) {
            if (flags.isSkipSvc()) {
                ret = __system_property_update(pi, value, strlen(value));
            } else {
                ret = system_property_set(name, value);
            }
            LOGD("resetprop: update prop [%s]: [%s] by %s\n", name, value, msg);
        } else {
            if (flags.isSkipSvc()) {
                ret = __system_property_add(name, strlen(name), value, strlen(value));
            } else {
                ret = system_property_set(name, value);
            }
            LOGD("resetprop: create prop [%s]: [%s] by %s\n", name, value, msg);
        }

        if (ret)
            LOGW("resetprop: setprop error\n");

        return ret;
    }

    string get_prop(const char *name, PropFlags flags) {
        if (!check_legal_property_name(name))
            return "";

        if (flags.isContext()) {
            auto val = __system_property_get_context(name) ?: "";
            LOGD("resetprop: getcontext [%s]: [%s]\n", name, val);
            return val;
        }

        string val;
        auto pi = system_property_find(name);
        if (pi == nullptr)
            return val;
        auto cb = prop_to_string(val);
        read_prop_with_cb(pi, &cb);
        LOGD("resetprop: getprop [%s]: [%s]\n", name, val.data());

        if (val.empty() && flags.isPersist() && strncmp(name, "persist.", 8) == 0)
            val = persist_getprop(name);
        if (val.empty())
            LOGD("resetprop: prop [%s] does not exist\n", name);
        return val;
    }

    void print_props(PropFlags flags) {
        prop_list list;
        prop_collector collector(list);
        system_property_foreach(read_prop_with_cb, &collector);
        if (flags.isPersist())
            persist_getprops(&collector);
        for (auto &[key, val] : list) {
            const char *v = flags.isContext() ?
                    (__system_property_get_context(key.data()) ?: "") :
                    val.data();
            printf("[%s]: [%s]\n", key.data(), v);
        }
    }

    // Not an error when something is deleted
    int delete_prop(const char *name, PropFlags flags) {
        LOGD("resetprop: delete prop [%s]\n", name);

        int ret = __system_property_delete(name, true);
        if (flags.isPersist() && str_starts(name, "persist.")) {
            if (persist_deleteprop(name))
                ret = 0;
        }
        return ret;
    }

    void load_file(const char *filename, PropFlags flags) {
        LOGD("resetprop: Parse prop file [%s]\n", filename);
        parse_prop_file(filename, [&](auto key, auto val) -> bool {
            set_prop(key.data(), val.data(), flags);
            return true;
        });
    }
};

static resetprop *get_impl() {
    static resetprop impl;
    return &impl;
}

int resetprop_main(int argc, char *argv[]) {
    PropFlags flags;
    char *argv0 = argv[0];

    --argc;
    ++argv;

    // Parse flags and -- options
    while (argc && argv[0][0] == '-') {
        for (int idx = 1; true; ++idx) {
            switch (argv[0][idx]) {
            case '-':
                if (argv[0] == "--file"sv) {
                    flags.setLoadFile();
                } else if (argv[0] == "--delete"sv) {
                    flags.setDelete();
                } else if (argv[0] == "--help"sv) {
                    usage(argv0);
                }
            case 'v':
                set_log_level_state(LogLevel::Debug, true);
                continue;
            case 'p':
                flags.setPersist();
                continue;
            case 'n':
                flags.setSkipSvc();
                continue;
            case 'Z':
                flags.setContext();
                continue;
            case '\0':
                break;
            case 'h':
            default:
                usage(argv0);
            }
            break;
        }
        --argc;
        ++argv;
    }

    resetprop *impl = get_impl();

    if (flags.isDelete()) {
        if (argc != 1)
            usage(argv0);
        return impl->delete_prop(argv[0], flags);
    }

    if (flags.isLoadFile()) {
        if (argc != 1)
            usage(argv0);
        impl->load_file(argv[0], flags);
        return 0;
    }

    switch (argc) {
    case 0:
        impl->print_props(flags);
        return 0;
    case 1: {
        string val = impl->get_prop(argv[0], flags);
        if (val.empty())
            return 1;
        printf("%s\n", val.data());
        return 0;
    }
    case 2:
        return impl->set_prop(argv[0], argv[1], flags);
    default:
        usage(argv0);
    }
}

/***********************************
 * Implementation of top-level APIs
 ***********************************/

string get_prop(const char *name, bool persist) {
    PropFlags flags;
    if (persist) flags.setPersist();
    return get_impl()->get_prop(name, flags);
}

int delete_prop(const char *name, bool persist) {
    PropFlags flags;
    if (persist) flags.setPersist();
    return get_impl()->delete_prop(name, flags);
}

int set_prop(const char *name, const char *value, bool skip_svc) {
    PropFlags flags;
    if (skip_svc) flags.setSkipSvc();
    return get_impl()->set_prop(name, value, flags);
}

void load_prop_file(const char *filename, bool skip_svc) {
    PropFlags flags;
    if (skip_svc) flags.setSkipSvc();
    get_impl()->load_file(filename, flags);
}
