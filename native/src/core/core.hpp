#pragma once

#include <string>
#include <vector>
#include <cxx.h>

extern bool RECOVERY_MODE;
extern std::atomic<ino_t> pkg_xml_ino;

std::string find_preinit_device();
void unlock_blocks();
void reboot();
std::string read_certificate(int fd, int version = -1);

// Module stuffs
void handle_modules();
void load_modules();
void disable_modules();
void remove_modules();
void exec_module_scripts(const char *stage);

// Scripting
void exec_script(const char *script);
void exec_common_scripts(const char *stage);
void exec_module_scripts(const char *stage, const std::vector<std::string_view> &modules);
void install_apk(const char *apk);
void uninstall_pkg(const char *pkg);
void clear_pkg(const char *pkg, int user_id);
[[noreturn]] void install_module(const char *file);

// System properties
rust::String get_prop_rs(const char *name, bool persist);
std::string get_prop(const char *name, bool persist = false);
int delete_prop(const char *name, bool persist = false);
int set_prop(const char *name, const char *value, bool skip_svc = false);
void load_prop_file(const char *filename, bool skip_svc = false);
