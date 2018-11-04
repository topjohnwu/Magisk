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
#include "utils.h"
#include "su.h"

#define AM_PATH "/system/bin/app_process", "/system/bin", "com.android.commands.am.Am"

static const char *get_command(const struct su_request *to) {
	if (to->command[0])
		return to->command;
	if (to->shell[0])
		return to->shell;
	return DEFAULT_SHELL;
}

static void silent_run(const char *args[]) {
	if (fork_dont_care())
		return;
	int zero = open("/dev/zero", O_RDONLY | O_CLOEXEC);
	xdup2(zero, 0);
	int null = open("/dev/null", O_WRONLY | O_CLOEXEC);
	xdup2(null, 1);
	xdup2(null, 2);
	setenv("CLASSPATH", "/system/framework/am.jar", 1);
	execv(args[0], (char **) args);
	PLOGE("exec am");
	_exit(EXIT_FAILURE);
}

static void setup_user(char *user, struct su_info *info) {
	switch (info->cfg[SU_MULTIUSER_MODE]) {
		case MULTIUSER_MODE_OWNER_ONLY:
		case MULTIUSER_MODE_OWNER_MANAGED:
			sprintf(user, "%d", 0);
			break;
		case MULTIUSER_MODE_USER:
			sprintf(user, "%d", info->uid / 100000);
			break;
	}
}

void app_log(struct su_context *ctx) {
	char user[8];
	setup_user(user, ctx->info);

	char fromUid[8];
	sprintf(fromUid, "%d",
			ctx->info->cfg[SU_MULTIUSER_MODE] == MULTIUSER_MODE_OWNER_MANAGED ?
			ctx->info->uid % 100000 : ctx->info->uid);

	char toUid[8];
	sprintf(toUid, "%d", ctx->req.uid);

	char pid[8];
	sprintf(pid, "%d", ctx->pid);

	char policy[2];
	sprintf(policy, "%d", ctx->info->access.policy);

	const char *cmd[] = {
		AM_PATH, "broadcast",
		"-a", "android.intent.action.BOOT_COMPLETED",
		"-p", ctx->info->str[SU_MANAGER],
		"-f", "0x00000020",
		"--user", user,
		"--es", "action", "log",
		"--ei", "from.uid", fromUid,
		"--ei", "to.uid", toUid,
		"--ei", "pid", pid,
		"--ei", "policy", policy,
		"--es", "command", get_command(&ctx->req),
		"--ez", "notify", ctx->info->access.notify ? "true" : "false",
		nullptr
	};
	silent_run(cmd);
}

void app_notify(struct su_context *ctx) {
	char user[8];
	setup_user(user, ctx->info);

	char fromUid[8];
	sprintf(fromUid, "%d",
			ctx->info->cfg[SU_MULTIUSER_MODE] == MULTIUSER_MODE_OWNER_MANAGED ?
			ctx->info->uid % 100000 : ctx->info->uid);

	char policy[2];
	sprintf(policy, "%d", ctx->info->access.policy);

	const char *cmd[] = {
			AM_PATH, "broadcast",
			"-a", "android.intent.action.BOOT_COMPLETED",
			"-p", ctx->info->str[SU_MANAGER],
			"-f", "0x00000020",
			"--user", user,
			"--es", "action", "notify",
			"--ei", "from.uid", fromUid,
			"--ei", "policy", policy,
			nullptr
	};
	silent_run(cmd);
}

void app_connect(const char *socket, struct su_info *info) {
	char user[8];
	setup_user(user, info);
	const char *cmd[] = {
		AM_PATH, "broadcast",
		"-a", "android.intent.action.BOOT_COMPLETED",
		"-p", info->str[SU_MANAGER],
		"-f", "0x00000020",
		"--user", user,
		"--es", "action", "request",
		"--es", "socket", socket,
		nullptr
	};
	silent_run(cmd);
}

void socket_send_request(int fd, struct su_info *info) {
	write_key_token(fd, "uid", info->uid);
	write_string_be(fd, "eof");
}
