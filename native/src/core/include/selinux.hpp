#pragma once

#include <base.hpp>

int setcon(const char *con);
int getfilecon(const char *path, byte_data con);
int lgetfilecon(const char *path, byte_data con);
int fgetfilecon(int fd, byte_data con);
int setfilecon(const char *path, const char *con);
int lsetfilecon(const char *path, const char *con);
int fsetfilecon(int fd, const char *con);
int getfilecon_at(int dirfd, const char *name, byte_data con);
void setfilecon_at(int dirfd, const char *name, const char *con);

void restorecon();
void restore_tmpcon();
