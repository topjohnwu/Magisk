#pragma once

void freecon(char *con);
int setcon(const char *con);
int getfilecon(const char *path, char **con);
int lgetfilecon(const char *path, char **con);
int fgetfilecon(int fd, char **con);
int setfilecon(const char *path, const char *con);
int lsetfilecon(const char *path, const char *con);
int fsetfilecon(int fd, const char *con);
void getfilecon_at(int dirfd, const char *name, char **con);
void setfilecon_at(int dirfd, const char *name, const char *con);

//void enable_selinux();
void restorecon();
void restore_tmpcon();
