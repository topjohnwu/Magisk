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

#define CALL_PROVIDER \
"/system/bin/app_process", "/system/bin", "com.android.commands.content.Content", \
"call", "--uri", nullptr, "--user", nullptr, "--method"

#define content_exec_info(info, ...) {\
	const char *cmd[] = { CALL_PROVIDER, __VA_ARGS__, nullptr }; \
	exec_content_cmd(cmd, info); \
}

#define content_exec(...) content_exec_info(ctx.info.get(), __VA_ARGS__)

#define ex(s) "--extra", s

#define get_user(info) \
(info->cfg[SU_MULTIUSER_MODE] == MULTIUSER_MODE_USER \
? info->uid / 100000 \
: 0)

#define get_uid(info) \
(info->cfg[SU_MULTIUSER_MODE] == MULTIUSER_MODE_OWNER_MANAGED \
? info->uid % 100000 \
: info->uid)

static const char *get_command(const su_request *to) {
	if (to->command[0])
		return to->command;
	if (to->shell[0])
		return to->shell;
	return DEFAULT_SHELL;
}

static void exec_content_cmd(const char **args, const su_info *info) {
	char target[128];
	sprintf(target, "content://%s.provider", info->str[SU_MANAGER].data());
	char user[4];
	sprintf(user, "%d", get_user(info));

	// Fill in non static arguments
	args[5] = target;
	args[7] = user;

	exec_t exec {
		.pre_exec = [] {
			int null = xopen("/dev/null", O_WRONLY | O_CLOEXEC);
			dup2(null, STDOUT_FILENO);
			dup2(null, STDERR_FILENO);
			setenv("CLASSPATH", "/system/framework/content.jar", 1);
		},
		.fork = fork_dont_care,
		.argv = args
	};
	exec_command(exec);
}

#define LOG_BODY \
"log", \
ex(fromUid), ex(toUid), ex(pid), ex(policy), \
ex(command.data()), ex(notify)

void app_log(const su_context &ctx) {
	char fromUid[32];
	sprintf(fromUid, "from.uid:i:%d", get_uid(ctx.info));

	char toUid[32];
	sprintf(toUid, "to.uid:i:%d", ctx.req.uid);

	char pid[16];
	sprintf(pid, "pid:i:%d", ctx.pid);

	char policy[16];
	sprintf(policy, "policy:i:%d", ctx.info->access.policy);

	string command("command:s:");
	command += get_command(&ctx.req);

	char notify[16];
	sprintf(notify, "notify:b:%s", ctx.info->access.notify ? "true" : "false");

	content_exec(LOG_BODY)
}

#define NOTIFY_BODY \
"notify", ex(fromUid), ex(policy)

void app_notify(const su_context &ctx) {
	char fromUid[32];
	sprintf(fromUid, "from.uid:i:%d", get_uid(ctx.info));

	char policy[16];
	sprintf(policy, "policy:i:%d", ctx.info->access.policy);

	content_exec(NOTIFY_BODY)
}

#define SOCKET_BODY \
"request", ex(sock)

void app_socket(const char *socket, const shared_ptr<su_info> &info) {
	char sock[128];
	sprintf(sock, "socket:s:%s", socket);
	content_exec_info(info.get(), SOCKET_BODY)
}

void socket_send_request(int fd, const shared_ptr<su_info> &info) {
	write_key_token(fd, "uid", info->uid);
	write_string_be(fd, "eof");
}
