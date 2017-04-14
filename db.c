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

#include "magisk.h"
#include "su.h"

struct callback_data_t {
	struct su_context *ctx;
	policy_t policy;
};

static int database_callback(void *v, int argc, char **argv, char **azColName){
	struct callback_data_t *data = (struct callback_data_t *)v;
	policy_t policy = INTERACTIVE;
	int i;
	time_t until = 0;
	for(i = 0; i < argc; i++) {
		if (strcmp(azColName[i], "policy") == 0) {
			if (argv[i] != NULL) {
				policy = atoi(argv[i]);
			}
		}
		else if (strcmp(azColName[i], "until") == 0) {
			if (argv[i] != NULL) {
				until = atol(argv[i]);
			}
		}
	}

	if (policy == DENY) {
		data->policy = DENY;
		return -1;
	} else if (policy == ALLOW && (until == 0 || until > time(NULL))) {
		data->policy = ALLOW;
		// even though we allow, continue, so we can see if there's another policy
		// that denies...
	}

	return 0;
}

policy_t database_check(struct su_context *ctx) {
	sqlite3 *db = NULL;
	
	char query[512];
	snprintf(query, sizeof(query), "select policy, until from policies where uid=%d", ctx->from.uid);
	int ret = sqlite3_open_v2(ctx->user.database_path, &db, SQLITE_OPEN_READONLY, NULL);
	if (ret) {
		LOGE("sqlite3 open failure: %d", ret);
		sqlite3_close(db);
		return INTERACTIVE;
	}
	
	char *err = NULL;
	struct callback_data_t data;
	data.ctx = ctx;
	data.policy = INTERACTIVE;
	ret = sqlite3_exec(db, query, database_callback, &data, &err);
	sqlite3_close(db);
	if (err != NULL) {
		LOGE("sqlite3_exec: %s", err);
		return DENY;
	}

	return data.policy;
}
