#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern void (*freecon)(char * con);
extern int (*setcon)(const char * con);
extern int (*getfilecon)(const char *path, char ** con);
extern int (*lgetfilecon)(const char *path, char ** con);
extern int (*setfilecon)(const char *path, const char * con);
extern int (*lsetfilecon)(const char *path, const char * con);

void dload_selinux();
void restorecon();

#ifdef __cplusplus
}
#endif
