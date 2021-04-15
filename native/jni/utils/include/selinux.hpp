#pragma once

// selinuxfs paths
#define SELINUX_MNT         "/sys/fs/selinux"
#define SELINUX_ENFORCE     SELINUX_MNT "/enforce"
#define SELINUX_POLICY      SELINUX_MNT "/policy"
#define SELINUX_LOAD        SELINUX_MNT "/load"
#define SELINUX_CONTEXT     SELINUX_MNT "/context"
#define SELINUX_VERSION     SELINUX_MNT "/policyvers"

// sepolicy paths
#define PLAT_POLICY_DIR     "/system/etc/selinux/"
#define VEND_POLICY_DIR     "/vendor/etc/selinux/"
#define PROD_POLICY_DIR     "/product/etc/selinux/"
#define ODM_POLICY_DIR      "/odm/etc/selinux/"
#define SYSEXT_POLICY_DIR   "/system_ext/etc/selinux/"
#define SPLIT_PLAT_CIL      PLAT_POLICY_DIR "plat_sepolicy.cil"

// Unconstrained domain the daemon and root processes run in
#define SEPOL_PROC_DOMAIN   "magisk"
// Highly constrained domain, sole purpose is to connect to daemon
#define SEPOL_CLIENT_DOMAIN "magisk_client"
// Unconstrained file type that anyone can access
#define SEPOL_FILE_TYPE     "magisk_file"
// Special file type to allow clients to transit to client domain automatically
#define SEPOL_EXEC_TYPE     "magisk_exec"

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

void enable_selinux();
void restorecon();
void restore_tmpcon();
