#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cil/cil.h>

#include <utils.hpp>
#include <logging.hpp>
#include <stream.hpp>
#include <magiskpolicy.hpp>

#include "sepolicy.h"

int load_policydb(const char *file) {
	LOGD("Load policy from: %s\n", file);
	if (magisk_policydb)
		destroy_policydb();

	struct policy_file pf;
	policy_file_init(&pf);
	pf.fp = xfopen(file, "re");
	pf.type = PF_USE_STDIO;

	magisk_policydb = static_cast<policydb_t *>(xmalloc(sizeof(policydb_t)));
	if (policydb_init(magisk_policydb) || policydb_read(magisk_policydb, &pf, 0)) {
		LOGE("Fail to load policy from %s\n", file);
		return 1;
	}

	fclose(pf.fp);
	return 0;
}

#define SHALEN 64
static bool cmp_sha256(const char *a, const char *b) {
	char id_a[SHALEN] = {0};
	char id_b[SHALEN] = {0};
	if (int fd = xopen(a, O_RDONLY | O_CLOEXEC); fd >= 0) {
		xread(fd, id_a, SHALEN);
		close(fd);
	} else {
		return false;
	}

	if (int fd = xopen(b, O_RDONLY | O_CLOEXEC); fd >= 0) {
		xread(fd, id_b, SHALEN);
		close(fd);
	} else {
		return false;
	}
	LOGD("%s=[%.*s]\n", a, SHALEN, id_a);
	LOGD("%s=[%.*s]\n", b, SHALEN, id_b);
	return memcmp(id_a, id_b, SHALEN) == 0;
}

static bool check_precompiled(const char *precompiled) {
	bool ok = false;
	const char *actual_sha;
	char compiled_sha[128];

	actual_sha = PLAT_POLICY_DIR "plat_and_mapping_sepolicy.cil.sha256";
	if (access(actual_sha, R_OK) == 0) {
		ok = true;
		sprintf(compiled_sha, "%s.plat_and_mapping.sha256", precompiled);
		if (!cmp_sha256(actual_sha, compiled_sha))
			return false;
	}

	actual_sha = PLAT_POLICY_DIR "plat_sepolicy_and_mapping.sha256";
	if (access(actual_sha, R_OK) == 0) {
		ok = true;
		sprintf(compiled_sha, "%s.plat_sepolicy_and_mapping.sha256", precompiled);
		if (!cmp_sha256(actual_sha, compiled_sha))
			return false;
	}

	actual_sha = PROD_POLICY_DIR "product_sepolicy_and_mapping.sha256";
	if (access(actual_sha, R_OK) == 0) {
		ok = true;
		sprintf(compiled_sha, "%s.product_sepolicy_and_mapping.sha256", precompiled);
		if (!cmp_sha256(actual_sha, compiled_sha) != 0)
			return false;
	}

	actual_sha = SYSEXT_POLICY_DIR "system_ext_sepolicy_and_mapping.sha256";
	if (access(actual_sha, R_OK) == 0) {
		ok = true;
		sprintf(compiled_sha, "%s.system_ext_sepolicy_and_mapping.sha256", precompiled);
		if (!cmp_sha256(actual_sha, compiled_sha) != 0)
			return false;
	}

	return ok;
}

int load_split_cil() {
	const char *odm_pre = ODM_POLICY_DIR "precompiled_sepolicy";
	const char *vend_pre = VEND_POLICY_DIR "precompiled_sepolicy";
	if (access(odm_pre, R_OK) == 0 && check_precompiled(odm_pre))
		return load_policydb(odm_pre);
	else if (access(vend_pre, R_OK) == 0 && check_precompiled(vend_pre))
		return load_policydb(vend_pre);
	else
		return compile_split_cil();
}

static void load_cil(struct cil_db *db, const char *file) {
	char *addr;
	size_t size;
	mmap_ro(file, addr, size);
	cil_add_file(db, (char *) file, addr, size);
	LOGD("cil_add [%s]\n", file);
	munmap(addr, size);
}

int compile_split_cil() {
	char path[128], plat_ver[10];
	struct cil_db *db = nullptr;
	sepol_policydb_t *pdb = nullptr;
	FILE *f;
	int policy_ver;
	const char *cil_file;

	cil_db_init(&db);
	cil_set_mls(db, 1);
	cil_set_multiple_decls(db, 1);
	cil_set_disable_neverallow(db, 1);
	cil_set_target_platform(db, SEPOL_TARGET_SELINUX);
	cil_set_attrs_expand_generated(db, 0);

	f = xfopen(SELINUX_VERSION, "re");
	fscanf(f, "%d", &policy_ver);
	fclose(f);
	cil_set_policy_version(db, policy_ver);

	// Get mapping version
	f = xfopen(VEND_POLICY_DIR "plat_sepolicy_vers.txt", "re");
	fscanf(f, "%s", plat_ver);
	fclose(f);

	// plat
	load_cil(db, SPLIT_PLAT_CIL);

	sprintf(path, PLAT_POLICY_DIR "mapping/%s.cil", plat_ver);
	load_cil(db, path);

	// product
	sprintf(path, PROD_POLICY_DIR "mapping/%s.cil", plat_ver);
	if (access(path, R_OK) == 0)
		load_cil(db, path);

	cil_file = PROD_POLICY_DIR "product_sepolicy.cil";
	if (access(cil_file, R_OK) == 0)
		load_cil(db, cil_file);

	// vendor
	cil_file = VEND_POLICY_DIR "nonplat_sepolicy.cil";
	if (access(cil_file, R_OK) == 0)
		load_cil(db, cil_file);

	cil_file = VEND_POLICY_DIR "plat_pub_versioned.cil";
	if (access(cil_file, R_OK) == 0)
		load_cil(db, cil_file);

	cil_file = VEND_POLICY_DIR "vendor_sepolicy.cil";
	if (access(cil_file, R_OK) == 0)
		load_cil(db, cil_file);

	// odm
	cil_file = ODM_POLICY_DIR "odm_sepolicy.cil";
	if (access(cil_file, R_OK) == 0)
		load_cil(db, cil_file);

	if (cil_compile(db))
		return 1;
	if (cil_build_policydb(db, &pdb))
		return 1;

	cil_db_destroy(&db);
	magisk_policydb = &pdb->p;
	return 0;
}

int dump_policydb(const char *file) {
	uint8_t *data;
	size_t len;

	{
		auto fp = make_stream_fp<byte_stream>(data, len);
		struct policy_file pf;
		policy_file_init(&pf);
		pf.type = PF_USE_STDIO;
		pf.fp = fp.get();
		if (policydb_write(magisk_policydb, &pf)) {
			LOGE("Fail to create policy image\n");
			return 1;
		}
	}

	int fd = xopen(file, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd < 0)
		return 1;
	xwrite(fd, data, len);

	close(fd);
	free(data);
	return 0;
}

void destroy_policydb() {
	if (magisk_policydb) {
		policydb_destroy(magisk_policydb);
		free(magisk_policydb);
		magisk_policydb = nullptr;
	}
}
