#pragma once

#include <string>
#include <map>
#include <cxx.h>

struct prop_cb {
    virtual void exec(const char *name, const char *value, uint32_t serial) = 0;
};

using prop_list = std::map<std::string, std::string>;

struct prop_collector : prop_cb {
    explicit prop_collector(prop_list &list) : list(list) {}
    void exec(const char *name, const char *value, uint32_t) override {
        list.insert({name, value});
    }
private:
    prop_list &list;
};

// System properties
rust::String get_prop_rs(const char *name, bool persist);
std::string get_prop(const char *name, bool persist = false);
int delete_prop(const char *name, bool persist = false);
int set_prop(const char *name, const char *value, bool skip_svc = false);
void load_prop_file(const char *filename, bool skip_svc = false);

static inline void prop_cb_exec(prop_cb &cb, const char *name, const char *value, uint32_t serial) {
    cb.exec(name, value, serial);
}
