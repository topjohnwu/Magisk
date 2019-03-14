#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void (*freecon)(char *con);
extern int (*setcon)(const char *con);
extern int (*getfilecon)(const char *path, char **con);
extern int (*lgetfilecon)(const char *path, char **con);
extern int (*fgetfilecon)(int fd, char **con);
extern int (*setfilecon)(const char *path, const char *con);
extern int (*lsetfilecon)(const char *path, const char *con);
extern int (*fsetfilecon)(int fd, const char *con);
void getfilecon_at(int dirfd, const char *name, char **con);
void setfilecon_at(int dirfd, const char *name, const char *con);

void selinux_builtin_impl();
void dload_selinux();
void restorecon();

#ifdef __cplusplus
}
#endif
