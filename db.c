/*
** Copyright 2017, John Wu (@topjohnwu)
** Copyright 2013, Koushik Dutta (@koush)
**
*/

#include <stdlib.h>
#include <stdio.h>
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
				target = ctx->info->pkg_name;
			entry = argv[i];
		} else if (strcmp(azColName[i], "value") == 0) {
			value = argv[i];
		}
	}
	LOGD("su_db: query %s=[%s]\n", entry, value);
	strcpy(target, value);
	return 0;
}

void database_check(struct su_context *ctx) {
	sqlite3 *db = NULL;
	int ret;
	char buffer[PATH_MAX], *err = NULL;
	const char *base = access("/data/user_de", F_OK) == 0 ? "/data/user_de" : "/data/user";

	// Set default values
	ctx->info->root_access = ROOT_ACCESS_APPS_AND_ADB;
	ctx->info->multiuser_mode = MULTIUSER_MODE_OWNER_ONLY;
	ctx->info->mnt_ns = NAMESPACE_MODE_REQUESTER;
	strcpy(ctx->info->pkg_name, "???");  /* bad string so it doesn't exist */

	// Open database
	ret = sqlite3_open_v2(DATABASE_PATH, &db, SQLITE_OPEN_READONLY, NULL);
	if (ret) {
		LOGE("sqlite3 open failure: %s\n", sqlite3_errstr(ret));
		sqlite3_close(db);
		goto stat_requester;
	}

	// Query for strings
	sqlite3_exec(db, "SELECT key, value FROM strings", strings_callback, ctx, &err);
	if (err)
		LOGE("sqlite3_exec: %s\n", err);
	err = NULL;

	// Query for settings
	sqlite3_exec(db, "SELECT key, value FROM settings", settings_callback, ctx, &err);
	if (err)
		LOGE("sqlite3_exec: %s\n", err);
	err = NULL;

	// Query for policy
	int uid = -1;
	switch (ctx->info->multiuser_mode) {
	case MULTIUSER_MODE_OWNER_ONLY:
		if (ctx->info->uid / 100000) {
			uid = -1;
			ctx->info->policy = DENY;
			ctx->notify = 0;
		} else {
			uid = ctx->info->uid;
		}
		break;
	case MULTIUSER_MODE_OWNER_MANAGED:
		uid = ctx->info->uid % 100000;
		break;
	case MULTIUSER_MODE_USER:
		uid = ctx->info->uid;
		break;
	}

	sprintf(buffer, "SELECT policy, until FROM policies WHERE uid=%d", uid);
	sqlite3_exec(db, buffer, policy_callback, ctx, &err);
	if (err)
		LOGE("sqlite3_exec: %s\n", err);

	sqlite3_close(db);

stat_requester:
	// We prefer the original name
	sprintf(buffer, "%s/0/" JAVA_PACKAGE_NAME, base);
	if (stat(buffer, &ctx->info->st) == 0) {
		strcpy(ctx->info->pkg_name, JAVA_PACKAGE_NAME);
	} else {
		sprintf(buffer, "%s/0/%s", base, ctx->info->pkg_name);
		if (stat(buffer, &ctx->info->st) == -1) {
			LOGE("su: cannot find requester");
			ctx->info->policy = DENY;
			ctx->notify = 0;
		}
	}
}
