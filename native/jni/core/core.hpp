#pragma once

#include <string>
#include <vector>

extern bool RECOVERY_MODE;
extern int DAEMON_STATE;
extern std::atomic<ino_t> pkg_xml_ino;

// Daemon state
enum : int {
    STATE_NONE,
    STATE_POST_FS_DATA,
    STATE_POST_FS_DATA_DONE,
    STATE_LATE_START_DONE,
    STATE_BOOT_COMPLETE
};

void unlock_blocks();
void reboot();
void start_log_daemon();
void setup_logfile(bool reset);
std::string read_certificate(int fd, int version = -1);

// Module stuffs
void handle_modules();
void magic_mount();
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
