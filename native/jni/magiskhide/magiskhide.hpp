#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <functional>
#include <map>
#include <set>

#include <daemon.hpp>

#define SIGTERMTHRD SIGUSR1
#define ISOLATED_MAGIC "isolated"

// CLI entries
int launch_magiskhide();
int stop_magiskhide();
int add_list(int client);
int rm_list(int client);
void ls_list(int client);
[[noreturn]] void test_proc_monitor();

// Process monitoring
[[noreturn]] void proc_monitor();

// Utility functions
void crawl_procfs(const std::function<bool (int)> &fn);
void crawl_procfs(DIR *dir, const std::function<bool (int)> &fn);
bool hide_enabled();
void update_uid_map();

// Hide policies
void hide_daemon(int pid);
void hide_unmount(int pid = getpid());
void hide_sensitive_props();
void hide_late_sensitive_props();

extern pthread_mutex_t hide_state_lock;
extern std::map<int, std::vector<std::string_view>> uid_proc_map;

enum {
    LAUNCH_MAGISKHIDE,
    STOP_MAGISKHIDE,
    ADD_HIDELIST,
    RM_HIDELIST,
    LS_HIDELIST,
    HIDE_STATUS,
};

enum {
    HIDE_IS_ENABLED = DAEMON_LAST,
    HIDE_NOT_ENABLED,
    HIDE_ITEM_EXIST,
    HIDE_ITEM_NOT_EXIST,
    HIDE_NO_NS,
    HIDE_INVALID_PKG
};
