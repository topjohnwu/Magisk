#pragma once

#include <string>
#include <vector>

extern bool RECOVERY_MODE;
extern std::atomic<ino_t> pkg_xml_ino;

std::string find_preinit_device();
void unlock_blocks();
void reboot();
void start_log_daemon();
void setup_logfile(bool reset);
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
