#pragma once

#include <string>
#include <functional>

int setprop(const char *name, const char *value, bool prop_svc = true);
std::string getprop(const char *name, bool persist = false);
void getprops(void (*callback)(const char *, const char *, void *),
        void *cookie = nullptr, bool persist = false);
int delprop(const char *name, bool persist = false);
void load_prop_file(const char *filename, bool prop_svc = true);
