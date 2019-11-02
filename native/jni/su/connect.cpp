#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#include <daemon.h>
#include <utils.h>
#include <logging.h>

#include "su.h"

using namespace std;

enum connect_mode {
	UNINITIALIZED = 0,
	MODE_ACTIVITY,
	MODE_BROADCAST_COMPONENT,
	MODE_BROADCAST_PACKAGE
};

static connect_mode current_mode = UNINITIALIZED;

#define START_ACTIVITY \
"/system/bin/app_process", "/system/bin", "com.android.commands.am.Am", \
"start", "-n", nullptr, "--user", nullptr, "-f", "0x18000020", "-a"

// 0x18000020 = FLAG_ACTIVITY_NEW_TASK|FLAG_ACTIVITY_MULTIPLE_TASK|FLAG_INCLUDE_STOPPED_PACKAGES

#define START_BROADCAST \
"/system/bin/app_process", "/system/bin", "com.android.commands.am.Am", \
"broadcast", "-n", nullptr, "--user", nullptr, "-f", "0x00000020", \
"-a", "android.intent.action.REBOOT", "--es", "action"

#define START_BROADCAST_PKG \
"/system/bin/app_process", "/system/bin", "com.android.commands.am.Am", \
"broadcast", "-p", nullptr, "--user", nullptr, "-f", "0x00000020", \
"-a", "android.intent.action.REBOOT", "--es", "action"

// 0x00000020 = FLAG_INCLUDE_STOPPED_PACKAGES

#define am_app_info(info, ...) \
if (current_mode == MODE_BROADCAST_PACKAGE) { \
	const char *cmd[] = { START_BROADCAST_PKG, __VA_ARGS__, nullptr }; \
	exec_am_cmd(cmd, info); \
} else if (current_mode == MODE_BROADCAST_COMPONENT) { \
	const char *cmd[] = { START_BROADCAST, __VA_ARGS__, nullptr }; \
	exec_am_cmd(cmd, info); \
} else { \
	const char *cmd[] = { START_ACTIVITY, __VA_ARGS__, nullptr }; \
	exec_am_cmd(cmd, info); \
}

#define am_app(...) am_app_info(ctx.info.get(), __VA_ARGS__)

static const char *get_command(const su_request *to) {
	if (to->command[0])
		return to->command;
	if (to->shell[0])
		return to->shell;
	return DEFAULT_SHELL;
}

static void get_user(char *user, const su_info *info) {
	sprintf(user, "%d",
			info->cfg[SU_MULTIUSER_MODE] == MULTIUSER_MODE_USER
			? info->uid / 100000
			: 0);
}

static void get_uid(char *uid, const su_info *info) {
	sprintf(uid, "%d",
			info->cfg[SU_MULTIUSER_MODE] == MULTIUSER_MODE_OWNER_MANAGED
			? info->uid % 100000
			: info->uid);
}

static void exec_am_cmd(const char **args, const su_info *info) {
	char target[128];
	if (args[3][0] == 'b') {
		// Broadcast
		if (args[4][1] == 'p') {
			// Broadcast to package (receiver can be obfuscated)
			strcpy(target, info->str[SU_MANAGER].data());
		} else {
			// a.h is the broadcast receiver
			sprintf(target, "%s/a.h", info->str[SU_MANAGER].data());
		}
	} else {
		// a.m is the activity
		sprintf(target, "%s/a.m", info->str[SU_MANAGER].data());
	}
	char user[8];
	get_user(user, info);

	// Fill in non static arguments
	args[5] = target;
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
"--ez", "notify", ctx.info->access.notify ? "true" : "false"

void app_log(const su_context &ctx) {
	char fromUid[8];
	get_uid(fromUid, ctx.info.get());

	char toUid[8];
	sprintf(toUid, "%d", ctx.req.uid);

	char pid[8];
	sprintf(pid, "%d", ctx.pid);

	char policy[2];
	sprintf(policy, "%d", ctx.info->access.policy);

	am_app(LOG_BODY)
}

#define NOTIFY_BODY \
"notify", \
"--ei", "from.uid", fromUid, \
"--ei", "policy", policy

void app_notify(const su_context &ctx) {
	char fromUid[8];
	get_uid(fromUid, ctx.info.get());

	char policy[2];
	sprintf(policy, "%d", ctx.info->access.policy);

	am_app(NOTIFY_BODY)
}

#define SOCKET_BODY \
"request", \
"--es", "socket", socket

void app_socket(const char *socket, const shared_ptr<su_info> &info) {
	am_app_info(info.get(), SOCKET_BODY)
}

#define TEST_BODY \
"test", "--ei", "mode", mode, nullptr

void broadcast_test(int client) {
	if (client >= 0) {
		// Make it not uninitialized
		current_mode = MODE_ACTIVITY;
		write_int(client, 0);
		close(client);
	}

	su_info info;
	get_db_settings(info.cfg);
	get_db_strings(info.str);
	validate_manager(info.str[SU_MANAGER], 0, &info.mgr_st);

	char mode[2];
	{
		sprintf(mode, "%d", MODE_BROADCAST_PACKAGE);
		const char *cmd[] = { START_BROADCAST_PKG, TEST_BODY };
		exec_am_cmd(cmd, &info);
	}
	{
		sprintf(mode, "%d", MODE_BROADCAST_COMPONENT);
		const char *cmd[] = { START_BROADCAST, TEST_BODY };
		exec_am_cmd(cmd, &info);
	}
}

void broadcast_ack(int client) {
	int mode = read_int(client);
	if (mode < 0) {
		// Return connection mode to client
		write_int(client, current_mode);
	} else {
		if (mode > current_mode) {
			LOGD("* Use connect mode [%d] for su request and notify\n", mode);
			current_mode = static_cast<connect_mode>(mode);
		}
		write_int(client, 0);
	}
	close(client);
}

void socket_send_request(int fd, const shared_ptr<su_info> &info) {
	write_key_token(fd, "uid", info->uid);
	write_string_be(fd, "eof");
}
