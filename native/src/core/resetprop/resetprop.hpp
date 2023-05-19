#pragma once

#include <string>
#include <map>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <api/_system_properties.h>

struct prop_cb {
    virtual void exec(const char *name, const char *value) = 0;
};

using prop_list = std::map<std::string, std::string>;

struct prop_collector : prop_cb {
    explicit prop_collector(prop_list &list) : list(list) {}
    void exec(const char *name, const char *value) override {
        list.insert({name, value});
    }
private:
    prop_list &list;
};

std::string persist_get_prop(const char *name);
void persist_get_props(prop_cb *prop_cb);
bool persist_delete_prop(const char *name);
bool persist_set_prop(const char *name, const char *value);
