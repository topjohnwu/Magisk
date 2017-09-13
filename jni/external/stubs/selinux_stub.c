#include <stdbool.h>
#include <selinux/avc.h>
#include <selinux/context.h>
#include <selinux/get_context_list.h>
#include <selinux/get_default_type.h>
#include <selinux/label.h>
#include <selinux/restorecon.h>
#include <selinux/selinux.h>
int is_selinux_enabled(void) { return 0; }
int is_selinux_mls_enabled(void) { return 0; }
void freecon(char * con) { }
void freeconary(char ** con) { }
int getcon(char ** con) { return 0; }
int getcon_raw(char ** con) { return 0; }
int setcon(const char * con) { return 0; }
int setcon_raw(const char * con) { return 0; }
int getpidcon(pid_t pid, char ** con) { return 0; }
int getpidcon_raw(pid_t pid, char ** con) { return 0; }
int getprevcon(char ** con) { return 0; }
int getprevcon_raw(char ** con) { return 0; }
int getexeccon(char ** con) { return 0; }
int getexeccon_raw(char ** con) { return 0; }
int setexeccon(const char * con) { return 0; }
int setexeccon_raw(const char * con) { return 0; }
int getfscreatecon(char ** con) { return 0; }
int getfscreatecon_raw(char ** con) { return 0; }
int setfscreatecon(const char * context) { return 0; }
int setfscreatecon_raw(const char * context) { return 0; }
int getkeycreatecon(char ** con) { return 0; }
int getkeycreatecon_raw(char ** con) { return 0; }
int setkeycreatecon(const char * context) { return 0; }
int setkeycreatecon_raw(const char * context) { return 0; }
int getsockcreatecon(char ** con) { return 0; }
int getsockcreatecon_raw(char ** con) { return 0; }
int setsockcreatecon(const char * context) { return 0; }
int setsockcreatecon_raw(const char * context) { return 0; }
int getfilecon(const char *path, char ** con) { return 0; }
int getfilecon_raw(const char *path, char ** con) { return 0; }
int lgetfilecon(const char *path, char ** con) { return 0; }
int lgetfilecon_raw(const char *path, char ** con) { return 0; }
int fgetfilecon(int fd, char ** con) { return 0; }
int fgetfilecon_raw(int fd, char ** con) { return 0; }
int setfilecon(const char *path, const char * con) { return 0; }
int setfilecon_raw(const char *path, const char * con) { return 0; }
int lsetfilecon(const char *path, const char * con) { return 0; }
int lsetfilecon_raw(const char *path, const char * con) { return 0; }
int fsetfilecon(int fd, const char * con) { return 0; }
int fsetfilecon_raw(int fd, const char * con) { return 0; }
int getpeercon(int fd, char ** con) { return 0; }
int getpeercon_raw(int fd, char ** con) { return 0; }
void selinux_set_callback(int type, union selinux_callback cb) { }
int security_compute_av(const char * scon,
					const char * tcon,
					security_class_t tclass,
					access_vector_t requested,
					struct av_decision *avd) { return 0; }
int security_compute_av_raw(const char * scon,
					const char * tcon,
					security_class_t tclass,
					access_vector_t requested,
					struct av_decision *avd) { return 0; }
int security_compute_av_flags(const char * scon,
					const char * tcon,
					security_class_t tclass,
					access_vector_t requested,
					struct av_decision *avd) { return 0; }
int security_compute_av_flags_raw(const char * scon,
					const char * tcon,
					security_class_t tclass,
					access_vector_t requested,
					struct av_decision *avd) { return 0; }
int security_compute_create(const char * scon,
					const char * tcon,
					security_class_t tclass,
					char ** newcon) { return 0; }
int security_compute_create_raw(const char * scon,
					const char * tcon,
					security_class_t tclass,
					char ** newcon) { return 0; }
int security_compute_create_name(const char * scon,
					const char * tcon,
					security_class_t tclass,
					const char *objname,
					char ** newcon) { return 0; }
int security_compute_create_name_raw(const char * scon,
					const char * tcon,
					security_class_t tclass,
					const char *objname,
					char ** newcon) { return 0; }
int security_compute_relabel(const char * scon,
					const char * tcon,
					security_class_t tclass,
					char ** newcon) { return 0; }
int security_compute_relabel_raw(const char * scon,
					const char * tcon,
					security_class_t tclass,
					char ** newcon) { return 0; }
int security_compute_member(const char * scon,
					const char * tcon,
					security_class_t tclass,
					char ** newcon) { return 0; }
int security_compute_member_raw(const char * scon,
					const char * tcon,
					security_class_t tclass,
					char ** newcon) { return 0; }
int security_compute_user(const char * scon,
				const char *username,
				char *** con) { return 0; }
int security_compute_user_raw(const char * scon,
				const char *username,
				char *** con) { return 0; }
int security_load_policy(void *data, size_t len) { return 0; }
int security_get_initial_context(const char *name,
					char ** con) { return 0; }
int security_get_initial_context_raw(const char *name,
						char ** con) { return 0; }
int selinux_mkload_policy(int preservebools) { return 0; }
int selinux_init_load_policy(int *enforce) { return 0; }
int security_set_boolean_list(size_t boolcnt,
					SELboolean * boollist, int permanent) { return 0; }
int security_load_booleans(char *path) { return 0; }
int security_check_context(const char * con) { return 0; }
int security_check_context_raw(const char * con) { return 0; }
int security_canonicalize_context(const char * con,
					char ** canoncon) { return 0; }
int security_canonicalize_context_raw(const char * con,
						char ** canoncon) { return 0; }
int security_getenforce(void) { return 0; }
int security_setenforce(int value) { return 0; }
int security_deny_unknown(void) { return 0; }
int security_disable(void) { return 0; }
int security_policyvers(void) { return 0; }
int security_get_boolean_names(char ***names, int *len) { return 0; }
int security_get_boolean_pending(const char *name) { return 0; }
int security_get_boolean_active(const char *name) { return 0; }
int security_set_boolean(const char *name, int value) { return 0; }
int security_commit_booleans(void) { return 0; }
int selinux_set_mapping(struct security_class_mapping *map) { return 0; }
security_class_t mode_to_security_class(mode_t mode) { return 0; }
security_class_t string_to_security_class(const char *name) { return 0; }
const char *security_class_to_string(security_class_t cls) { return 0; }
const char *security_av_perm_to_string(security_class_t tclass,
						access_vector_t perm) { return 0; }
access_vector_t string_to_av_perm(security_class_t tclass,
					const char *name) { return 0; }
int security_av_string(security_class_t tclass,
				access_vector_t av, char **result) { return 0; }
void print_access_vector(security_class_t tclass, access_vector_t av) { }
void set_matchpathcon_printf(void (*f) (const char *fmt, ...)) { }
void set_matchpathcon_invalidcon(int (*f) (const char *path,
						unsigned lineno,
						char *context)) { }
void set_matchpathcon_canoncon(int (*f) (const char *path,
						unsigned lineno,
						char **context)) { }
void set_matchpathcon_flags(unsigned int flags) { }
int matchpathcon_init(const char *path) { return 0; }
int matchpathcon_init_prefix(const char *path, const char *prefix) { return 0; }
void matchpathcon_fini(void) { }
int realpath_not_final(const char *name, char *resolved_path) { return 0; }
int matchpathcon(const char *path,
			mode_t mode, char ** con) { return 0; }
int matchpathcon_index(const char *path,
				  mode_t mode, char ** con) { return 0; }
int matchpathcon_filespec_add(ino_t ino, int specind, const char *file) { return 0; }
void matchpathcon_filespec_destroy(void) { }
void matchpathcon_filespec_eval(void) { }
void matchpathcon_checkmatches(char *str) { }
int matchmediacon(const char *media, char ** con) { return 0; }
int selinux_getenforcemode(int *enforce) { return 0; }
char *selinux_boolean_sub(const char *boolean_name) { return 0; }
int selinux_getpolicytype(char **policytype) { return 0; }
const char *selinux_policy_root(void) { return 0; }
int selinux_set_policy_root(const char *rootpath) { return 0; }
const char *selinux_current_policy_path(void) { return 0; }
const char *selinux_binary_policy_path(void) { return 0; }
const char *selinux_failsafe_context_path(void) { return 0; }
const char *selinux_removable_context_path(void) { return 0; }
const char *selinux_default_context_path(void) { return 0; }
const char *selinux_user_contexts_path(void) { return 0; }
const char *selinux_file_context_path(void) { return 0; }
const char *selinux_file_context_homedir_path(void) { return 0; }
const char *selinux_file_context_local_path(void) { return 0; }
const char *selinux_file_context_subs_path(void) { return 0; }
const char *selinux_file_context_subs_dist_path(void) { return 0; }
const char *selinux_homedir_context_path(void) { return 0; }
const char *selinux_media_context_path(void) { return 0; }
const char *selinux_virtual_domain_context_path(void) { return 0; }
const char *selinux_virtual_image_context_path(void) { return 0; }
const char *selinux_lxc_contexts_path(void) { return 0; }
const char *selinux_x_context_path(void) { return 0; }
const char *selinux_sepgsql_context_path(void) { return 0; }
const char *selinux_openrc_contexts_path(void) { return 0; }
const char *selinux_openssh_contexts_path(void) { return 0; }
const char *selinux_snapperd_contexts_path(void) { return 0; }
const char *selinux_systemd_contexts_path(void) { return 0; }
const char *selinux_contexts_path(void) { return 0; }
const char *selinux_securetty_types_path(void) { return 0; }
const char *selinux_booleans_subs_path(void) { return 0; }
const char *selinux_booleans_path(void) { return 0; }
const char *selinux_customizable_types_path(void) { return 0; }
const char *selinux_users_path(void) { return 0; }
const char *selinux_usersconf_path(void) { return 0; }
const char *selinux_translations_path(void) { return 0; }
const char *selinux_colors_path(void) { return 0; }
const char *selinux_netfilter_context_path(void) { return 0; }
const char *selinux_path(void) { return 0; }
int selinux_check_access(const char * scon, const char * tcon, const char *tclass, const char *perm, void *auditdata) { return 0; }
int selinux_check_passwd_access(access_vector_t requested) { return 0; }
int checkPasswdAccess(access_vector_t requested) { return 0; }
int selinux_check_securetty_context(const char * tty_context) { return 0; }
void set_selinuxmnt(const char *mnt) { }
int selinuxfs_exists(void) { return 0; }
void fini_selinuxmnt(void) {}
int setexecfilecon(const char *filename, const char *fallback_type) { return 0; }
#ifndef DISABLE_RPM
int rpm_execcon(unsigned int verified,
			const char *filename,
			char *const argv[], char *const envp[]) { return 0; }
#endif
int is_context_customizable(const char * scontext) { return 0; }
int selinux_trans_to_raw_context(const char * trans,
					char ** rawp) { return 0; }
int selinux_raw_to_trans_context(const char * raw,
					char ** transp) { return 0; }
int selinux_raw_context_to_color(const char * raw,
					char **color_str) { return 0; }
int getseuserbyname(const char *linuxuser, char **seuser, char **level) { return 0; }
int getseuser(const char *username, const char *service,
			char **r_seuser, char **r_level) { return 0; }
int selinux_file_context_cmp(const char * a,
				const char * b) { return 0; }
int selinux_file_context_verify(const char *path, mode_t mode) { return 0; }
int selinux_lsetfilecon_default(const char *path) { return 0; }
void selinux_reset_config(void) { }
int avc_sid_to_context(security_id_t sid, char ** ctx) { return 0; }
int avc_sid_to_context_raw(security_id_t sid, char ** ctx) { return 0; }
int avc_context_to_sid(const char * ctx, security_id_t * sid) { return 0; }
int avc_context_to_sid_raw(const char * ctx, security_id_t * sid) { return 0; }
int sidget(security_id_t sid) { return 0; }
int sidput(security_id_t sid) { return 0; }
int avc_get_initial_sid(const char *name, security_id_t * sid) { return 0; }
int avc_init(const char *msgprefix,
		const struct avc_memory_callback *mem_callbacks,
		const struct avc_log_callback *log_callbacks,
		const struct avc_thread_callback *thread_callbacks,
		const struct avc_lock_callback *lock_callbacks) { return 0; }
int avc_open(struct selinux_opt *opts, unsigned nopts) { return 0; }
void avc_cleanup(void) { }
int avc_reset(void) { return 0; }
void avc_destroy(void) { }
int avc_has_perm_noaudit(security_id_t ssid,
			security_id_t tsid,
			security_class_t tclass,
			access_vector_t requested,
			struct avc_entry_ref *aeref, struct av_decision *avd) { return 0; }
int avc_has_perm(security_id_t ssid, security_id_t tsid,
			security_class_t tclass, access_vector_t requested,
			struct avc_entry_ref *aeref, void *auditdata) { return 0; }
void avc_audit(security_id_t ssid, security_id_t tsid,
			security_class_t tclass, access_vector_t requested,
			struct av_decision *avd, int result, void *auditdata) { }
int avc_compute_create(security_id_t ssid,
			security_id_t tsid,
			security_class_t tclass, security_id_t * newsid) { return 0; }
int avc_compute_member(security_id_t ssid,
			security_id_t tsid,
			security_class_t tclass, security_id_t * newsid) { return 0; }
int avc_add_callback(int (*callback)
			(uint32_t event, security_id_t ssid,
			security_id_t tsid, security_class_t tclass,
			access_vector_t perms,
			access_vector_t * out_retained),
			uint32_t events, security_id_t ssid,
			security_id_t tsid, security_class_t tclass,
			access_vector_t perms) { return 0; }
void avc_cache_stats(struct avc_cache_stats *stats) { }
void avc_av_stats(void) { }
void avc_sid_stats(void) { }
int avc_netlink_open(int blocking) { return 0; }
void avc_netlink_loop(void) { }
void avc_netlink_close(void) { }
int avc_netlink_acquire_fd(void) { return 0; }
void avc_netlink_release_fd(void) { }
int avc_netlink_check_nb(void) { return 0; }
int selinux_status_open(int fallback) { return 0; }
void selinux_status_close(void) { }
int selinux_status_updated(void) { return 0; }
int selinux_status_getenforce(void) { return 0; }
int selinux_status_policyload(void) { return 0; }
int selinux_status_deny_unknown(void) { return 0; }
context_t context_new(const char *s) { return 0; }
char *context_str(context_t c) { return 0; }
void context_free(context_t c) { }
const char *context_type_get(context_t c) { return 0; }
const char *context_range_get(context_t c) { return 0; }
const char *context_role_get(context_t c) { return 0; }
const char *context_user_get(context_t c) { return 0; }
int context_type_set(context_t c, const char *s) { return 0; }
int context_range_set(context_t c, const char *s) { return 0; }
int context_role_set(context_t c, const char *s) { return 0; }
int context_user_set(context_t c, const char *s) { return 0; }
int get_ordered_context_list(const char *user,
					char * fromcon,
					char *** list) { return 0; }
int get_ordered_context_list_with_level(const char *user,
					const char *level,
					char * fromcon,
					char *** list) { return 0; }
int get_default_context(const char *user,
					char * fromcon,
					char ** newcon) { return 0; }
int get_default_context_with_level(const char *user,
					const char *level,
					char * fromcon,
					char ** newcon) { return 0; }
int get_default_context_with_role(const char *user,
					const char *role,
					char * fromcon,
					char ** newcon) { return 0; }
int get_default_context_with_rolelevel(const char *user,
					const char *role,
					const char *level,
					char * fromcon,
					char ** newcon) { return 0; }
int query_user_context(char ** list,
				  char ** newcon) { return 0; }
int manual_user_enter_context(const char *user,
					char ** newcon) { return 0; }
const char *selinux_default_type_path(void) { return 0; }
int get_default_type(const char *role, char **type) { return 0; }
struct selabel_handle *selabel_open(unsigned int backend,
					const struct selinux_opt *opts,
					unsigned nopts) { return 0; }
void selabel_close(struct selabel_handle *handle) { }
int selabel_lookup(struct selabel_handle *handle, char **con,
			const char *key, int type) { return 0; }
int selabel_lookup_raw(struct selabel_handle *handle, char **con,
			const char *key, int type) { return 0; }
bool selabel_partial_match(struct selabel_handle *handle, const char *key) { return 0; }
int selabel_lookup_best_match(struct selabel_handle *rec, char **con,
				const char *key, const char **aliases, int type) { return 0; }
int selabel_lookup_best_match_raw(struct selabel_handle *rec, char **con,
				const char *key, const char **aliases, int type) { return 0; }
int selabel_digest(struct selabel_handle *rec,
				unsigned char **digest, size_t *digest_len,
				char ***specfiles, size_t *num_specfiles) { return 0; }
void selabel_stats(struct selabel_handle *handle) { }
int selinux_restorecon(const char *pathname,
					unsigned int restorecon_flags) { return 0; }
struct selabel_handle *selinux_restorecon_default_handle(void) { return 0; }
void selinux_restorecon_set_exclude_list(const char **exclude_list) { }
int selinux_restorecon_set_alt_rootpath(const char *alt_rootpath) { return 0; }
int selinux_restorecon_xattr(const char *pathname,
					unsigned int xattr_flags,
					struct dir_xattr ***xattr_list) { return 0; }
