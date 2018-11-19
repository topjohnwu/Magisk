/* resetprop.h - API for resetprop
 */

#pragma once

#include "CharArray.h"

int prop_exist(const char *name);
int setprop(const char *name, const char *value, const bool trigger = true);
CharArray getprop(const char *name, bool persist = false);
void getprop(void (*callback)(const char *, const char *, void *), void *cookie, bool persist = false);
int deleteprop(const char *name, bool persist = false);
int load_prop_file(const char *filename, const bool trigger = true);

