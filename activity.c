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
	if (fork())
		return;
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

static void setup_user(struct su_context *ctx, char* user) {
	switch (ctx->user.multiuser_mode) {
	case MULTIUSER_MODE_OWNER_ONLY:   /* Should already be denied if not owner */
	case MULTIUSER_MODE_OWNER_MANAGED:
		sprintf(user, "%d", 0);
		break;
	case MULTIUSER_MODE_USER:
		sprintf(user, "%d", ctx->user.android_user_id);
		break;
	}
}

void app_send_result(struct su_context *ctx, policy_t policy) {
	char fromUid[16];
	sprintf(fromUid, "%d", ctx->from.uid);

	char toUid[16];
	sprintf(toUid, "%d", ctx->to.uid);

	char pid[16];
	sprintf(pid, "%d", ctx->from.pid);

	char user[16];
	setup_user(ctx, user);

	char *result_command[] = {
		AM_PATH,
		ACTION_RESULT,
		"--ei",
		"from.uid",
		fromUid,
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
		"--user",
		user,
		NULL
	};
	silent_run(result_command);
}

void app_send_request(struct su_context *ctx) {
	char user[64];
	setup_user(ctx, user);

	char *request_command[] = {
		AM_PATH,
		ACTION_REQUEST,
		"--es",
		"socket",
		ctx->sock_path,
		"--user",
		user,
		NULL
	};

	silent_run(request_command);
}
