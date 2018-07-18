/*
** Copyright 2013, Koushik Dutta (@koush)
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sqlite3.h>
#include <time.h>

#include "su.h"

struct callback_data_t {
    struct su_context *ctx;
    policy_t policy;
};

static int database_callback(void *v, int argc, char **argv, char **azColName){
    struct callback_data_t *data = (struct callback_data_t *)v;
    int command_match = 0;
    policy_t policy = DENY;
    int i;
    time_t until = 0;
    for(i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "policy") == 0) {
            if (argv[i] == NULL) {
                policy = DENY;
            }
            if (strcmp(argv[i], "allow") == 0) {
                policy = ALLOW;
            }
            else if (strcmp(argv[i], "interactive") == 0) {
                policy = INTERACTIVE;
            }
            else {
                policy = DENY;
            }
        }
        else if (strcmp(azColName[i], "command") == 0) {
            // null or empty command means to match all commands (whitelist all from uid)
            command_match = argv[i] == NULL || strlen(argv[i]) == 0 || strcmp(argv[i], get_command(&(data->ctx->to))) == 0;
        }
        else if (strcmp(azColName[i], "until") == 0) {
            if (argv[i] != NULL) {
                until = atoi(argv[i]);
            }
        }
    }

    // check for command match
    if (command_match) {
        // also make sure this policy has not expired
        if (until == 0 || until > time(NULL)) {
            if (policy == DENY) {
                data->policy = DENY;
                return -1;
            }

            data->policy = ALLOW;
            // even though we allow, continue, so we can see if there's another policy
            // that denies...
        }
    }
    
    return 0;
}

policy_t database_check(struct su_context *ctx) {
    sqlite3 *db = NULL;
    
    char query[512];
    snprintf(query, sizeof(query), "select policy, until, command from uid_policy where uid=%d", ctx->from.uid);
    int ret = sqlite3_open_v2(ctx->user.database_path, &db, SQLITE_OPEN_READONLY, NULL);
    if (ret) {
        LOGE("sqlite3 open failure: %d", ret);
        sqlite3_close(db);
        return INTERACTIVE;
    }
    
    int result;
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
