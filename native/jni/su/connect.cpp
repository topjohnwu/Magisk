#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#include <daemon.h>
#include <utils.h>

#include "su.h"

using namespace std;

bool CONNECT_BROADCAST;

#define START_ACTIVITY \
"/system/bin/app_process", "/system/bin", "com.android.commands.am.Am", \
"start", "-n", nullptr, "--user", nullptr, "-f", "0x18000020", "-a"

// 0x18000020 = FLAG_ACTIVITY_NEW_TASK|FLAG_ACTIVITY_MULTIPLE_TASK|FLAG_INCLUDE_STOPPED_PACKAGES

#define START_BROADCAST \
"/system/bin/app_process", "/system/bin", "com.android.commands.am.Am", \
"broadcast", "-n", nullptr, "--user", nullptr, "-f", "0x00000020", \
"-a", "android.intent.action.REBOOT", "--es", "action"

// 0x00000020 = FLAG_INCLUDE_STOPPED_PACKAGES

static inline const char *get_command(const su_request *to) {
	if (to->command[0])
		return to->command;
	if (to->shell[0])
		return to->shell;
	return DEFAULT_SHELL;
}

static inline void get_user(char *user, const su_info *info) {
	sprintf(user, "%d",
			info->cfg[SU_MULTIUSER_MODE] == MULTIUSER_MODE_USER
			? info->uid / 100000
			: 0);
}

static inline void get_uid(char *uid, const su_info *info) {
	sprintf(uid, "%d",
			info->cfg[SU_MULTIUSER_MODE] == MULTIUSER_MODE_OWNER_MANAGED
			? info->uid % 100000
			: info->uid);
}

static void exec_am_cmd(const char **args, const su_info *info) {
	char component[128];
	sprintf(component, "%s/%s", info->str[SU_MANAGER].data(), args[3][0] == 'b' ? "a.h" : "a.m");
	char user[8];
	get_user(user, info);

	/* Fill in dynamic arguments */
	args[5] = component;
	args[7] = user;

	exec_t exec {
		.pre_exec = []() -> void {
			int null = xopen("/dev/null", O_WRONLY | O_CLOEXEC);
			dup2(null, STDOUT_FILENO);
			dup2(null, STDERR_FILENO);
			setenv("CLASSPATH", "/system/framework/am.jar", 1);
		},
		.fork = fork_dont_care,
		.argv = args
	};
	exec_command(exec);
}

#define LOG_BODY \
"log", \
"--ei", "from.uid", fromUid, \
"--ei", "to.uid", toUid, \
"--ei", "pid", pid, \
"--ei", "policy", policy, \
"--es", "command", get_command(&ctx.req), \
"--ez", "notify", ctx.info->access.notify ? "true" : "false", \
nullptr

void app_log(const su_context &ctx) {
	char fromUid[8];
	get_uid(fromUid, ctx.info.get());

	char toUid[8];
	sprintf(toUid, "%d", ctx.req.uid);

	char pid[8];
	sprintf(pid, "%d", ctx.pid);

	char policy[2];
	sprintf(policy, "%d", ctx.info->access.policy);

	if (CONNECT_BROADCAST) {
		const char *cmd[] = { START_BROADCAST, LOG_BODY };
		exec_am_cmd(cmd, ctx.info.get());
	} else {
		const char *cmd[] = { START_ACTIVITY, LOG_BODY };
		exec_am_cmd(cmd, ctx.info.get());
	}
}

#define NOTIFY_BODY \
"notify", \
"--ei", "from.uid", fromUid, \
"--ei", "policy", policy, \
nullptr

void app_notify(const su_context &ctx) {
	char fromUid[8];
	get_uid(fromUid, ctx.info.get());

	char policy[2];
	sprintf(policy, "%d", ctx.info->access.policy);

	if (CONNECT_BROADCAST) {
		const char *cmd[] = { START_BROADCAST, NOTIFY_BODY };
		exec_am_cmd(cmd, ctx.info.get());
	} else {
		const char *cmd[] = { START_ACTIVITY, NOTIFY_BODY };
		exec_am_cmd(cmd, ctx.info.get());
	}

}

void app_connect(const char *socket, const shared_ptr<su_info> &info) {
	const char *cmd[] = {
		START_ACTIVITY, "request",
		"--es", "socket", socket,
		nullptr
	};
	exec_am_cmd(cmd, info.get());
}

void broadcast_test() {
	su_info info;
	get_db_settings(info.cfg);
	get_db_strings(info.str);
	validate_manager(info.str[SU_MANAGER], 0, &info.mgr_st);

	const char *cmd[] = { START_BROADCAST, "test", nullptr };
	exec_am_cmd(cmd, &info);
}

void socket_send_request(int fd, const shared_ptr<su_info> &info) {
	write_key_token(fd, "uid", info->uid);
	write_string_be(fd, "eof");
}
