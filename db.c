/*
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

static int database_callback(void *v, int argc, char **argv, char **azColName) {
	struct su_context *ctx = (struct su_context *) v;
	policy_t policy = QUERY;
	time_t until = 0;
	for(int i = 0; i < argc; i++) {
		if (strcmp(azColName[i], "policy") == 0) {
			if (argv[i] != NULL) {
				policy = atoi(argv[i]);
			}
		} else if (strcmp(azColName[i], "until") == 0) {
			if (argv[i] != NULL) {
				until = atol(argv[i]);
			}
		}
	}

	if (policy == DENY)
		ctx->info->policy = DENY;
	else if (policy == ALLOW && (until == 0 || until > time(NULL)))
		ctx->info->policy = ALLOW;

	return 0;
}

void database_check(struct su_context *ctx) {
	sqlite3 *db = NULL;

	// Check if file is readable
	if (access(ctx->user.database_path, R_OK) == -1)
		return;
	
	char query[512];
	snprintf(query, sizeof(query), "SELECT policy, until FROM policies WHERE uid=%d", ctx->info->uid % 100000);
	int ret = sqlite3_open_v2(ctx->user.database_path, &db, SQLITE_OPEN_READONLY, NULL);
	if (ret) {
		LOGD("sqlite3 open failure: %s\n", sqlite3_errstr(ret));
		sqlite3_close(db);
		return;
	}
	
	char *err = NULL;
	ret = sqlite3_exec(db, query, database_callback, ctx, &err);
	sqlite3_close(db);
	if (err != NULL) {
		LOGE("sqlite3_exec: %s\n", err);
		ctx->info->policy = DENY;
	}
}
