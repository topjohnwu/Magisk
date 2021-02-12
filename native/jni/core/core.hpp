#pragma once

#include <string>
#include <vector>

extern bool RECOVERY_MODE;
extern int DAEMON_STATE;

void unlock_blocks();
void reboot();
void setup_logfile(bool reset);

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

