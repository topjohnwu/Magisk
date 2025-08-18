#pragma once

#include <string>
#include <cxx.h>

struct prop_info;

struct prop_callback {
    virtual ~prop_callback() = default;
    virtual void exec(const char *name, const char *value, uint32_t serial) = 0;
    void exec(Utf8CStr name, Utf8CStr value) {
        exec(name.data(), value.data(), INT_MAX);
    }
    void read(const prop_info *pi);
};

// System properties
std::string get_prop(const char *name, bool persist = false);
int delete_prop(const char *name, bool persist = false);
int set_prop(const char *name, const char *value, bool skip_svc = false);
void load_prop_file(const char *filename, bool skip_svc = false);

// Rust bindings
rust::String get_prop_rs(Utf8CStr name, bool persist);
static inline int set_prop_rs(Utf8CStr name, Utf8CStr value, bool skip_svc) {
    return set_prop(name.data(), value.data(), skip_svc);
}
static inline void load_prop_file_rs(Utf8CStr filename, bool skip_svc) {
    load_prop_file(filename.data(), skip_svc);
}