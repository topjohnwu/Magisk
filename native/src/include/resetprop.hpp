#pragma once

#include <string>
#include <functional>

std::string get_prop(const char *name, bool persist = false);
int delete_prop(const char *name, bool persist = false);
int set_prop(const char *name, const char *value, bool skip_svc = false);
void load_prop_file(const char *filename, bool skip_svc = false);
