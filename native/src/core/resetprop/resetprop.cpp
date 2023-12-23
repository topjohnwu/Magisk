#include <dlfcn.h>
#include <sys/types.h>
#include <vector>
#include <map>

#include <base.hpp>
#include <core.hpp>
#include <resetprop.hpp>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <api/_system_properties.h>
#include <system_properties/prop_info.h>

using namespace std;

#ifdef APPLET_STUB_MAIN
#define system_property_set             __system_property_set
#define system_property_read(...)
#define system_property_find            __system_property_find
#define system_property_read_callback   __system_property_read_callback
#define system_property_foreach         __system_property_foreach
#define system_property_wait            __system_property_wait
#else
static int (*system_property_set)(const char*, const char*);
static int (*system_property_read)(const prop_info*, char*, char*);
static const prop_info *(*system_property_find)(const char*);
static void (*system_property_read_callback)(
        const prop_info*, void (*)(void*, const char*, const char*, uint32_t), void*);
static int (*system_property_foreach)(void (*)(const prop_info*, void*), void*);
static bool (*system_property_wait)(const prop_info*, uint32_t, uint32_t*, const struct timespec*);
#endif

struct PropFlags {
    void setSkipSvc() { flags |= 1; }
    void setPersist() { flags |= (1 << 1); }
    void setContext() { flags |= (1 << 2); }
    void setPersistOnly() { flags |= (1 << 3); setPersist(); }
    void setWait() { flags |= (1 << 4); }
    bool isSkipSvc() const { return flags & 1; }
    bool isPersist() const { return flags & (1 << 1); }
    bool isContext() const { return flags & (1 << 2); }
    bool isPersistOnly() const { return flags & (1 << 3); }
    bool isWait() const { return flags & (1 << 4); }
private:
    uint32_t flags = 0;
};

[[noreturn]] static void usage(char* arg0) {
    fprintf(stderr,
R"EOF(resetprop - System Property Manipulation Tool

Usage: %s [flags] [arguments...]

Read mode arguments:
   (no arguments)    print all properties
   NAME              get property of NAME

Write mode arguments:
   NAME VALUE        set property NAME as VALUE
   -f,--file   FILE  load and set properties from FILE
   -d,--delete NAME  delete property

Wait mode arguments (toggled with -w):
    NAME             wait until property NAME changes
    NAME OLD_VALUE   if value of property NAME is not OLD_VALUE, get value
                     or else wait until property NAME changes

General flags:
   -h,--help         show this message
   -v                print verbose output to stderr
   -w                switch to wait mode

Read mode flags:
   -p      also read persistent props from storage
   -P      only read persistent props from storage
   -Z      get property context instead of value

Write mode flags:
   -n      set properties bypassing property_service
   -p      always write persistent prop changes to storage

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
        auto callback = [](void *cb, const char *name, const char *value, uint32_t serial) {
            static_cast<prop_cb*>(cb)->exec(name, value, serial);
        };
        system_property_read_callback(pi, callback, cb);
    } else {
        char name[PROP_NAME_MAX];
        char value[PROP_VALUE_MAX];
        name[0] = '\0';
        value[0] = '\0';
        system_property_read(pi, name, value);
        static_cast<prop_cb*>(cb)->exec(name, value, pi->serial);
    }
}

template<class StringType>
struct prop_to_string : prop_cb {
    void exec(const char *, const char *value, uint32_t s) override {
        val = value;
        serial = s;
    }
    StringType val;
    uint32_t serial;
};

template<> void prop_to_string<rust::String>::exec(const char *, const char *value, uint32_t s) {
    // We do not want to crash when values are not UTF-8
    val = rust::String::lossy(value);
    serial = s;
}

static int set_prop(const char *name, const char *value, PropFlags flags) {
    if (!check_legal_property_name(name))
        return 1;

    auto pi = const_cast<prop_info *>(__system_property_find(name));

    // Delete existing read-only properties if they are or will be long properties,
    // which cannot directly go through __system_property_update
    if (str_starts(name, "ro.")) {
        if (pi != nullptr && (pi->is_long() || strlen(value) >= PROP_VALUE_MAX)) {
            // Skip pruning nodes as we will add it back ASAP
            __system_property_delete(name, false);
            pi = nullptr;
        }
        flags.setSkipSvc();
    }

    const char *msg = flags.isSkipSvc() ? "direct modification" : "property_service";

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

    // When bypassing property_service, persistent props won't be stored in storage.
    // Explicitly handle this situation.
    if (ret == 0 && flags.isSkipSvc() && flags.isPersist() && str_starts(name, "persist.")) {
        ret = persist_set_prop(name, value) ? 0 : 1;
    }

    if (ret) {
        LOGW("resetprop: set prop error\n");
    }

    return ret;
}

template<class StringType>
static StringType get_prop(const char *name, PropFlags flags) {
    if (!check_legal_property_name(name))
        return {};

    prop_to_string<StringType> cb;

    if (flags.isContext()) {
        auto context = __system_property_get_context(name) ?: "";
        LOGD("resetprop: prop context [%s]: [%s]\n", name, context);
        cb.exec(name, context, -1);
        return cb.val;
    }

    if (!flags.isPersistOnly()) {
        if (auto pi = system_property_find(name)) {
            read_prop_with_cb(pi, &cb);
            LOGD("resetprop: get prop [%s]: [%s]\n", name, cb.val.c_str());
        }
    }

    if (cb.val.empty() && flags.isPersist() && str_starts(name, "persist."))
        persist_get_prop(name, cb);
    if (cb.val.empty())
        LOGD("resetprop: prop [%s] does not exist\n", name);

    return cb.val;
}

template<class StringType>
static StringType wait_prop(const char *name, const char *old_value) {
    if (!check_legal_property_name(name))
        return {};
    auto pi = system_property_find(name);
    if (!pi) {
        LOGD("resetprop: prop [%s] does not exist\n", name);
        return {};
    }

    prop_to_string<StringType> cb;
    read_prop_with_cb(pi, &cb);

    if (old_value == nullptr || cb.val == old_value) {
        LOGD("resetprop: waiting for prop [%s]\n", name);
        uint32_t new_serial;
        system_property_wait(pi, cb.serial, &new_serial, nullptr);
        read_prop_with_cb(pi, &cb);
    }

    LOGD("resetprop: get prop [%s]: [%s]\n", name, cb.val.c_str());
    return cb.val;
}

static void print_props(PropFlags flags) {
    prop_list list;
    prop_collector collector(list);
    if (!flags.isPersistOnly())
        system_property_foreach(read_prop_with_cb, &collector);
    if (flags.isPersist())
        persist_get_props(collector);
    for (auto &[key, val] : list) {
        const char *v = flags.isContext() ?
                (__system_property_get_context(key.data()) ?: "") :
                val.data();
        printf("[%s]: [%s]\n", key.data(), v);
    }
}

static int delete_prop(const char *name, PropFlags flags) {
    if (!check_legal_property_name(name))
        return 1;

    LOGD("resetprop: delete prop [%s]\n", name);

    int ret = __system_property_delete(name, true);
    if (flags.isPersist() && str_starts(name, "persist.")) {
        if (persist_delete_prop(name))
            ret = 0;
    }
    return ret;
}

static void load_file(const char *filename, PropFlags flags) {
    LOGD("resetprop: Parse prop file [%s]\n", filename);
    parse_prop_file(filename, [=](auto key, auto val) -> bool {
        set_prop(key.data(), val.data(), flags);
        return true;
    });
}

struct Initialize {
    Initialize() {
#ifndef APPLET_STUB_MAIN
#define DLOAD(name) (*(void **) &name = dlsym(RTLD_DEFAULT, "__" #name))
        // Load platform implementations
        DLOAD(system_property_set);
        DLOAD(system_property_read);
        DLOAD(system_property_find);
        DLOAD(system_property_read_callback);
        DLOAD(system_property_foreach);
        DLOAD(system_property_wait);
#undef DLOAD
        if (system_property_wait == nullptr) {
            // The platform API only exist on API 26+
            system_property_wait = __system_property_wait;
        }
#endif
        if (__system_properties_init()) {
            LOGE("resetprop: __system_properties_init error\n");
        }
    }
};

static void InitOnce() {
    static struct Initialize init;
}

#define consume_next(val)    \
if (argc != 2) usage(argv0); \
val = argv[1];               \
stop_parse = true;           \

int resetprop_main(int argc, char *argv[]) {
    PropFlags flags;
    char *argv0 = argv[0];
    set_log_level_state(LogLevel::Debug, false);

    const char *prop_file = nullptr;
    const char *prop_to_rm = nullptr;

    --argc;
    ++argv;

    // Parse flags and -- options
    while (argc && argv[0][0] == '-') {
        bool stop_parse = false;
        for (int idx = 1; true; ++idx) {
            switch (argv[0][idx]) {
            case '-':
                if (argv[0] == "--file"sv) {
                    consume_next(prop_file);
                } else if (argv[0] == "--delete"sv) {
                    consume_next(prop_to_rm);
                } else {
                    usage(argv0);
                }
                break;
            case 'd':
                consume_next(prop_to_rm);
                continue;
            case 'f':
                consume_next(prop_file);
                continue;
            case 'n':
                flags.setSkipSvc();
                continue;
            case 'p':
                flags.setPersist();
                continue;
            case 'P':
                flags.setPersistOnly();
                continue;
            case 'v':
                set_log_level_state(LogLevel::Debug, true);
                continue;
            case 'Z':
                flags.setContext();
                continue;
            case 'w':
                flags.setWait();
                continue;
            case '\0':
                break;
            default:
                usage(argv0);
            }
            break;
        }
        --argc;
        ++argv;
        if (stop_parse)
            break;
    }

    InitOnce();

    if (prop_to_rm) {
        return delete_prop(prop_to_rm, flags);
    }

    if (prop_file) {
        load_file(prop_file, flags);
        return 0;
    }

    if (flags.isWait()) {
        if (argc == 0) usage(argv0);
        auto val = wait_prop<string>(argv[0], argv[1]);
        if (val.empty())
            return 1;
        printf("%s\n", val.data());
        return 0;
    }

    switch (argc) {
    case 0:
        print_props(flags);
        return 0;
    case 1: {
        auto val = get_prop<string>(argv[0], flags);
        if (val.empty())
            return 1;
        printf("%s\n", val.data());
        return 0;
    }
    case 2:
        return set_prop(argv[0], argv[1], flags);
    default:
        usage(argv0);
    }
}

/***************
 * Public APIs
 ****************/

template<class StringType>
static StringType get_prop_impl(const char *name, bool persist) {
    InitOnce();
    PropFlags flags;
    if (persist) flags.setPersist();
    return get_prop<StringType>(name, flags);
}

rust::String get_prop_rs(const char *name, bool persist) {
    return get_prop_impl<rust::String>(name, persist);
}

string get_prop(const char *name, bool persist) {
    return get_prop_impl<string>(name, persist);
}

int delete_prop(const char *name, bool persist) {
    InitOnce();
    PropFlags flags;
    if (persist) flags.setPersist();
    return delete_prop(name, flags);
}

int set_prop(const char *name, const char *value, bool skip_svc) {
    InitOnce();
    PropFlags flags;
    if (skip_svc) flags.setSkipSvc();
    return set_prop(name, value, flags);
}

void load_prop_file(const char *filename, bool skip_svc) {
    InitOnce();
    PropFlags flags;
    if (skip_svc) flags.setSkipSvc();
    load_file(filename, flags);
}
