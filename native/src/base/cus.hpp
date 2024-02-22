#pragma once
#include <sys/wait.h>
#include <signal.h>

bool is_dir_exist(const char *s);
int bind_mount_(const char *from, const char *to);
int tmpfs_mount(const char *from, const char *to);
int setcurrent(const char *con);
std::string getcurrent();
bool selinux_enabled();
