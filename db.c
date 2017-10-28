/*
** Copyright 2017, John Wu (@topjohnwu)
** Copyright 2013, Koushik Dutta (@koush)
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sqlite3.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "magisk.h"
#include "su.h"

static int policy_callback(void *v, int argc, char **argv, char **azColName) {
	struct su_context *ctx = (struct su_context *) v;
	policy_t policy = QUERY;
	time_t until = 0;
	for (int i = 0; i < argc; i++) {
		if (strcmp(azColName[i], "policy") == 0)
			policy = atoi(argv[i]);
		else if (strcmp(azColName[i], "until") == 0)
			until = atol(argv[i]);
	}

	if (policy == DENY)
		ctx->info->policy = DENY;
	else if (policy == ALLOW && (until == 0 || until > time(NULL)))
		ctx->info->policy = ALLOW;

	LOGD("su_db: query policy=[%d]\n", ctx->info->policy);

	return 0;
}

static int settings_callback(void *v, int argc, char **argv, char **azColName) {
	struct su_context *ctx = (struct su_context *) v;
	int *target, value;
	char *entry;
	for (int i = 0; i < argc; ++i) {
		if (strcmp(azColName[i], "key") == 0) {
			if (strcmp(argv[i], ROOT_ACCESS_ENTRY) == 0)
				target = &ctx->info->root_access;
			else if (strcmp(argv[i], MULTIUSER_MODE_ENTRY) == 0)
				target = &ctx->info->multiuser_mode;
			else if (strcmp(argv[i], NAMESPACE_MODE_ENTRY) == 0)
				target = &ctx->info->mnt_ns;
			entry = argv[i];
		} else if (strcmp(azColName[i], "value") == 0) {
			value = atoi(argv[i]);
		}
	}
	LOGD("su_db: query %s=[%d]\n", entry, value);
	*target = value;
	return 0;
}

static int strings_callback(void *v, int argc, char **argv, char **azColName) {
	struct su_context *ctx = (struct su_context *) v;
	char *entry, *target, *value;
	for (int i = 0; i < argc; ++i) {
		if (strcmp(azColName[i], "key") == 0) {
			if (strcmp(argv[i], REQUESTER_ENTRY) == 0)
				target = ctx->path.pkg_name;
			entry = argv[i];
		} else if (strcmp(azColName[i], "value") == 0) {
			value = argv[i];
		}
	}
	LOGD("su_db: query %s=[%s]\n", entry, value);
	strcpy(target, value);
	return 0;
}

#define BASE_FMT  "/data/user%s/%d"
#define USE_MULTI(info) (info->uid / 100000 != 0 && info->multiuser_mode == MULTIUSER_MODE_USER)

void database_check(struct su_context *ctx) {
	sqlite3 *db = NULL;
	int ret;
	char buffer[PATH_MAX], *err = NULL;

	// Set default values
	ctx->info->root_access = ROOT_ACCESS_APPS_AND_ADB;
	ctx->info->multiuser_mode = MULTIUSER_MODE_OWNER_ONLY;
	ctx->info->mnt_ns = NAMESPACE_MODE_REQUESTER;
	strcpy(ctx->path.pkg_name, "???");  /* bad string so it doesn't exist */

	// Populate paths
	sprintf(ctx->path.base_path, BASE_FMT, "_de", 0);
	if (access(ctx->path.base_path, R_OK) == -1)
		sprintf(ctx->path.base_path, BASE_FMT, "", 0);

	sprintf(ctx->path.multiuser_path, BASE_FMT, "_de", ctx->info->uid / 100000);
	if (access(ctx->path.multiuser_path, R_OK) == -1)
		sprintf(ctx->path.multiuser_path, BASE_FMT, "", ctx->info->uid / 100000);

	// Open database
	sprintf(buffer, "%s/magisk.db", ctx->path.base_path);
	LOGD("su_db: open %s", buffer);
	ret = sqlite3_open_v2(buffer, &db, SQLITE_OPEN_READONLY, NULL);
	if (ret) {
		LOGD("sqlite3 open failure: %s\n", sqlite3_errstr(ret));
		sqlite3_close(db);
		goto stat_requester;
	}

	// Check multiuser mode settings
	sprintf(buffer, "SELECT key, value FROM settings WHERE key='%s'", MULTIUSER_MODE_ENTRY);
	sqlite3_exec(db, buffer, settings_callback, ctx, &err);
	if (err != NULL)
		LOGE("sqlite3_exec: %s\n", err);
	err = NULL;

	// Open database based on multiuser settings
	if (USE_MULTI(ctx->info)) {
		sqlite3_close(db);
		sprintf(buffer, "%s/magisk.db", ctx->path.multiuser_path);
		LOGD("su_db: open %s", buffer);
		ret = sqlite3_open_v2(buffer, &db, SQLITE_OPEN_READONLY, NULL);
		if (ret) {
			LOGD("sqlite3 open failure: %s\n", sqlite3_errstr(ret));
			sqlite3_close(db);
			goto stat_requester;
		}
	}

	// Read PKG name from DB
	strcpy(buffer, "SELECT key, value FROM strings");
	sqlite3_exec(db, buffer, strings_callback, ctx, &err);
	if (err != NULL)
		LOGE("sqlite3_exec: %s\n", err);
	err = NULL;

	// Query for policy
	sprintf(buffer, "SELECT policy, until FROM policies WHERE uid=%d", ctx->info->uid % 100000);
	sqlite3_exec(db, buffer, policy_callback, ctx, &err);
	if (err != NULL)
		LOGE("sqlite3_exec: %s\n", err);
	err = NULL;

	// Query for settings
	sprintf(buffer, "SELECT key, value FROM settings WHERE key!='%s'", MULTIUSER_MODE_ENTRY);
	sqlite3_exec(db, buffer, settings_callback, ctx, &err);
	if (err != NULL)
		LOGE("sqlite3_exec: %s\n", err);

	sqlite3_close(db);

stat_requester:
	// We prefer the orignal name
	sprintf(buffer, "%s/%s", USE_MULTI(ctx->info) ? ctx->path.multiuser_path : ctx->path.base_path, JAVA_PACKAGE_NAME);
	if (stat(buffer, &ctx->st) == -1) {
		sprintf(buffer, "%s/%s", USE_MULTI(ctx->info) ? ctx->path.multiuser_path : ctx->path.base_path, ctx->path.pkg_name);
		if (stat(buffer, &ctx->st) == -1) {
			LOGE("su: cannot find requester");
			ctx->info->policy = DENY;
			ctx->notify = 0;
		}
	} else {
		strcpy(ctx->path.pkg_name, JAVA_PACKAGE_NAME);
	}
}
