/*
** Copyright 2018, John Wu (@topjohnwu)
** Copyright 2010, Adam Shanks (@ChainsDD)
** Copyright 2008, Zinx Verituse (@zinxv)
**
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#include "magisk.h"
#include "daemon.h"
#include "su.h"

#define AM_PATH "/system/bin/app_process", "/system/bin", "com.android.commands.am.Am"

static char *get_command(const struct su_request *to) {
	if (to->command)
		return to->command;
	if (to->shell)
		return to->shell;
	return DEFAULT_SHELL;
}

static void silent_run(char * const args[]) {
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

static void setup_user(char *user) {
	switch (su_ctx->info->dbs.v[SU_MULTIUSER_MODE]) {
	case MULTIUSER_MODE_OWNER_ONLY:
	case MULTIUSER_MODE_OWNER_MANAGED:
		sprintf(user, "%d", 0);
		break;
	case MULTIUSER_MODE_USER:
		sprintf(user, "%d", su_ctx->info->uid / 100000);
		break;
	}
}

void app_log() {
	char user[8];
	setup_user(user);

	char fromUid[8];
	sprintf(fromUid, "%d",
			su_ctx->info->dbs.v[SU_MULTIUSER_MODE] == MULTIUSER_MODE_OWNER_MANAGED ?
			su_ctx->info->uid % 100000 : su_ctx->info->uid);

	char toUid[8];
	sprintf(toUid, "%d", su_ctx->to.uid);

	char pid[8];
	sprintf(pid, "%d", su_ctx->pid);

	char policy[2];
	sprintf(policy, "%d", su_ctx->info->access.policy);

	char *cmd[] = {
		AM_PATH, "broadcast",
		"-a", "android.intent.action.BOOT_COMPLETED",
		"-p", su_ctx->info->str.s[SU_MANAGER],
		"--user", user,
		"--es", "action", "log",
		"--ei", "from.uid", fromUid,
		"--ei", "to.uid", toUid,
		"--ei", "pid", pid,
		"--ei", "policy", policy,
		"--es", "command", get_command(&su_ctx->to),
		NULL
	};
	silent_run(cmd);
}

void app_connect(const char *socket) {
	char user[8];
	setup_user(user);
	char *cmd[] = {
		AM_PATH, "broadcast",
		"-a", "android.intent.action.BOOT_COMPLETED",
		"-p", su_ctx->info->str.s[SU_MANAGER],
		"--user", user,
		"--es", "action", "request",
		"--es", "socket", (char *) socket,
		NULL
	};
	silent_run(cmd);
}

void socket_send_request(int fd) {
	write_key_token(fd, "uid", su_ctx->info->uid);
	write_string_be(fd, "eof");
}

