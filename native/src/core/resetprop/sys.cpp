#include <dlfcn.h>

#include <base.hpp>
#include <core.hpp>

#include <api/system_properties.h>
#include <system_properties/prop_info.h>

using namespace std;

// This has to keep in sync with SysProp in mod.rs
struct SysProp {
    int (*set)(const char*, const char*);
    const prop_info *(*find)(const char*);
    void (*read_callback)(const prop_info*, void (*)(void*, const char*, const char*, uint32_t), void*);
    int (*foreach)(void (*)(const prop_info*, void*), void*);
    bool (*wait)(const prop_info*, uint32_t, uint32_t*, const timespec*);
};

extern "C" bool prop_info_is_long(const prop_info &info) {
    return info.is_long();
}

extern "C" SysProp get_sys_prop() {
    SysProp prop{};
#ifdef APPLET_STUB_MAIN
    // Use internal implementation
    prop.set = __system_property_set;
    prop.find = __system_property_find;
    prop.read_callback = __system_property_read_callback;
    prop.foreach = __system_property_foreach;
    prop.wait = __system_property_wait;
#else
#define DLOAD(name) (*(void **) &prop.name = dlsym(RTLD_DEFAULT, "__system_property_" #name))
    // Dynamic load platform implementation
    DLOAD(set);
    DLOAD(find);
    DLOAD(read_callback);
    DLOAD(foreach);
    DLOAD(wait);
#undef DLOAD
    if (prop.wait == nullptr) {
        // This platform API only exist on API 26+
        prop.wait = __system_property_wait;
    }
    if (prop.read_callback == nullptr) {
        // This platform API only exist on API 26+
        prop.read_callback = __system_property_read_callback;
    }
#endif
    if (__system_properties_init()) {
        LOGE("resetprop: __system_properties_init error\n");
    }
    return prop;
}
