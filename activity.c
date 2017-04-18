/*
** Copyright 2017, John Wu (@topjohnwu)
** Copyright 2010, Adam Shanks (@ChainsDD)
** Copyright 2008, Zinx Verituse (@zinxv)
**
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <stdarg.h>

#include "magisk.h"
#include "su.h"

/* intent actions */
#define ACTION_REQUEST "start", "-n", REQUESTOR "/" REQUESTOR_PREFIX ".RequestActivity"
#define ACTION_NOTIFY "start", "-n", REQUESTOR "/" REQUESTOR_PREFIX ".NotifyActivity"
#define ACTION_RESULT "broadcast", "-n", REQUESTOR "/" REQUESTOR_PREFIX ".SuReceiver"

#define AM_PATH "/system/bin/app_process", "/system/bin", "com.android.commands.am.Am"

static void silent_run(char* const args[]) {
	set_identity(0);
	pid_t pid;
	pid = fork();
	/* Parent */
	if (pid < 0) {
		PLOGE("fork");
	}
	else if (pid > 0) {
		return;
	}
	int zero = open("/dev/zero", O_RDONLY | O_CLOEXEC);
	dup2(zero, 0);
	int null = open("/dev/null", O_WRONLY | O_CLOEXEC);
	dup2(null, 1);
	dup2(null, 2);
	setenv("CLASSPATH", "/system/framework/am.jar", 1);
	execv(args[0], args);
	PLOGE("exec am");
	_exit(EXIT_FAILURE);
}

static int get_owner_login_user_args(struct su_context *ctx, char* user, int user_len) {
	int needs_owner_login_prompt = 0;
	
	if (ctx->user.multiuser_mode == MULTIUSER_MODE_OWNER_MANAGED) {
		if (0 != ctx->user.android_user_id) {
			needs_owner_login_prompt = 1;
		}
		snprintf(user, user_len, "0");
	}
	else if (ctx->user.multiuser_mode == MULTIUSER_MODE_USER) {
		snprintf(user, user_len, "%d", ctx->user.android_user_id);
	}
	else if (ctx->user.multiuser_mode == MULTIUSER_MODE_NONE) {
		user[0] = '\0';
	}
	else {
		snprintf(user, user_len, "0");
	}
	
	return needs_owner_login_prompt;
}

void app_send_result(struct su_context *ctx, policy_t policy) {
	// char binary_version[256];
	// sprintf(binary_version, "%d", VERSION_CODE);

	char uid[256];
	sprintf(uid, "%d", ctx->from.uid);

	char toUid[256];
	sprintf(toUid, "%d", ctx->to.uid);

	char pid[256];
	sprintf(pid, "%d", ctx->from.pid);

	char user[64];
	get_owner_login_user_args(ctx, user, sizeof(user));

	if (0 != ctx->user.android_user_id) {
		char android_user_id[256];
		sprintf(android_user_id, "%d", ctx->user.android_user_id);

		char *user_result_command[] = {
			AM_PATH,
			ACTION_RESULT,
			"--ei",
			"from.uid",
			uid,
			"--ei",
			"to.uid",
			toUid,
			"--ei",
			"pid",
			pid,
			"--es",
			"command",
			get_command(&ctx->to),
			"--es",
			"action",
			policy == ALLOW ? "allow" : "deny",
			user[0] ? "--user" : NULL,
			android_user_id,
			NULL
		};
		silent_run(user_result_command);
	}

	char *result_command[] = {
		AM_PATH,
		ACTION_RESULT,
		"--ei",
		"from.uid",
		uid,
		"--ei",
		"to.uid",
		toUid,
		"--ei",
		"pid",
		pid,
		"--es",
		"command",
		get_command(&ctx->to),
		"--es",
		"action",
		policy == ALLOW ? "allow" : "deny",
		user[0] ? "--user" : NULL,
		user,
		NULL
	};
	silent_run(result_command);
}

void app_send_request(struct su_context *ctx) {
	// if su is operating in MULTIUSER_MODEL_OWNER,
	// and the user requestor is not the owner,
	// the owner needs to be notified of the request.
	// so there will be two activities shown.
	char user[64];
	int needs_owner_login_prompt = get_owner_login_user_args(ctx, user, sizeof(user));

	if (needs_owner_login_prompt) {
		char uid[256];
		sprintf(uid, "%d", ctx->from.uid);

		char android_user_id[256];
		sprintf(android_user_id, "%d", ctx->user.android_user_id);

		// in multiuser mode, the owner gets the su prompt
		char *notify_command[] = {
			AM_PATH,
			ACTION_NOTIFY,
			"--ei",
			"caller_uid",
			uid,
			"--user",
			android_user_id,
			NULL
		};

		silent_run(notify_command);
	}

	char *request_command[] = {
		AM_PATH,
		ACTION_REQUEST,
		"--es",
		"socket",
		ctx->sock_path,
		user[0] ? "--user" : NULL,
		user,
		NULL
	};

	silent_run(request_command);
}
