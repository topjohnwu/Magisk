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

void database_check(struct su_context *ctx) {
	sqlite3 *db = NULL;
	int ret;
	char query[512], *err = NULL;

	// Set default values
	ctx->info->root_access = ROOT_ACCESS_APPS_AND_ADB;
	ctx->info->multiuser_mode = MULTIUSER_MODE_OWNER_ONLY;
	ctx->info->mnt_ns = NAMESPACE_MODE_REQUESTER;
	ctx->info->policy = QUERY;

	// First query the from app data
	// Check if file is readable
	if (access(APP_DATA_PATH REQUESTOR_DATABASE_PATH, R_OK) == -1)
		return;

	// Open database
	ret = sqlite3_open_v2(APP_DATA_PATH REQUESTOR_DATABASE_PATH, &db, SQLITE_OPEN_READONLY, NULL);
	if (ret) {
		LOGD("sqlite3 open failure: %s\n", sqlite3_errstr(ret));
		sqlite3_close(db);
		return;
	}

	// Check multiuser mode settings
	snprintf(query, sizeof(query), "SELECT key, value FROM settings WHERE key='%s'", MULTIUSER_MODE_ENTRY);
	sqlite3_exec(db, query, settings_callback, ctx, &err);

	err = NULL;

	if (ctx->user.android_user_id != 0 && ctx->info->multiuser_mode == MULTIUSER_MODE_USER) {
		sqlite3_close(db);
		// Check if file is readable
		if (access(ctx->user.database_path, R_OK) == -1)
			return;

		// Open database
		ret = sqlite3_open_v2(ctx->user.database_path, &db, SQLITE_OPEN_READONLY, NULL);
		if (ret) {
			LOGD("sqlite3 open failure: %s\n", sqlite3_errstr(ret));
			sqlite3_close(db);
			return;
		}
	}

	// Query for policy
	snprintf(query, sizeof(query), "SELECT policy, until FROM policies WHERE uid=%d", ctx->info->uid % 100000);
	sqlite3_exec(db, query, policy_callback, ctx, &err);
	if (err != NULL) {
		LOGE("sqlite3_exec: %s\n", err);
		return;
	}

	err = NULL;

	// Query for settings
	snprintf(query, sizeof(query), "SELECT key, value FROM settings WHERE key!='%s'", MULTIUSER_MODE_ENTRY);
	sqlite3_exec(db, query, settings_callback, ctx, &err);
	if (err != NULL) {
		LOGE("sqlite3_exec: %s\n", err);
		return;
	}

	sqlite3_close(db);
}
