#pragma once

#include <string>
#include <vector>
#include <functional>

extern bool RECOVERY_MODE;
extern int DAEMON_STATE;
extern bool zygisk_enabled;

void unlock_blocks();
void reboot();
void start_log_daemon();
void setup_logfile(bool reset);
void magisk_logging();

// Thread pool
void exec_task(std::function<void()> &&task);

// Module stuffs
void handle_modules();
void magic_mount();
void disable_modules();
void remove_modules();
void exec_module_scripts(const char *stage);

// Scripting
void exec_script(const char *script);
void exec_common_scripts(const char *stage);
void exec_module_scripts(const char *stage, const std::vector<std::string> &module_list);
void install_apk(const char *apk);
[[noreturn]] void install_module(const char *file);
