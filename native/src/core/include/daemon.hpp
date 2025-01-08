#pragma once

#include <base.hpp>

const char *get_magisk_tmp();
void install_apk(rust::Utf8CStr apk);
void uninstall_pkg(rust::Utf8CStr pkg);

// Rust bindings
static inline rust::Utf8CStr get_magisk_tmp_rs() { return get_magisk_tmp(); }
static inline rust::String resolve_preinit_dir_rs(rust::Utf8CStr base_dir) {
    return resolve_preinit_dir(base_dir.c_str());
}
