/* resetprop.h - API for resetprop
 */

#pragma once

#include <string>
#include <functional>

int prop_exist(const char *name);
int setprop(const char *name, const char *value, bool trigger = true);
std::string getprop(const char *name, bool persist = false);
void getprop(void (*callback)(const char *, const char *, void *), void *cookie, bool persist = false);
int deleteprop(const char *name, bool persist = false);
void load_prop_file(const char *filename, bool trigger = true);
