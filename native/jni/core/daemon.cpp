#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/mount.h>

#include <magisk.h>
#include <utils.h>
#include <daemon.h>
#include <selinux.h>
#include <db.h>
#include <resetprop.h>
#include <flags.h>

int SDK_INT = -1;
bool RECOVERY_MODE = false;
static struct stat self_st;

static void verify_client(int client, pid_t pid) {
	// Verify caller is the same as server
	char path[32];
	sprintf(path, "/proc/%d/exe", pid);
	struct stat st;
	if (stat(path, &st) || st.st_dev != self_st.st_dev || st.st_ino != self_st.st_ino) {
		close(client);
		pthread_exit(nullptr);
	}
}

static void *request_handler(void *args) {
	int client = reinterpret_cast<intptr_t>(args);

	struct ucred credential;
	get_client_cred(client, &credential);
	if (credential.uid != 0)
		verify_client(client, credential.pid);

	int req = read_int(client);
	switch (req) {
	case MAGISKHIDE:
	case POST_FS_DATA:
	case LATE_START:
	case BOOT_COMPLETE:
	case SQLITE_CMD:
		if (credential.uid != 0) {
			write_int(client, ROOT_REQUIRED);
			close(client);
			return nullptr;
		}
	default:
		break;
	}

	switch (req) {
	case MAGISKHIDE:
		magiskhide_handler(client);
		break;
	case SUPERUSER:
		su_daemon_handler(client, &credential);
		break;
	case CHECK_VERSION:
		write_string(client, MAGISK_VERSION ":MAGISK");
		close(client);
		break;
	case CHECK_VERSION_CODE:
		write_int(client, MAGISK_VER_CODE);
		close(client);
		break;
	case POST_FS_DATA:
		post_fs_data(client);
		break;
	case LATE_START:
		late_start(client);
		break;
	case BOOT_COMPLETE:
		boot_complete(client);
		break;
	case SQLITE_CMD:
		exec_sql(client);
		break;
	case REMOVE_MODULES:
		if (credential.uid == UID_SHELL || credential.uid == UID_ROOT) {
			remove_modules();
			write_int(client, 0);
		} else {
			write_int(client, 1);
		}
		close(client);
		break;
	default:
		close(client);
		break;
	}
	return nullptr;
}

static void main_daemon() {
	android_logging();

	int fd = xopen("/dev/null", O_WRONLY);
	xdup2(fd, STDOUT_FILENO);
	xdup2(fd, STDERR_FILENO);
	if (fd > STDERR_FILENO)
		close(fd);
	fd = xopen("/dev/zero", O_RDONLY);
	xdup2(fd, STDIN_FILENO);
	if (fd > STDERR_FILENO)
		close(fd);
	close(fd);

	setsid();
	setcon("u:r:" SEPOL_PROC_DOMAIN ":s0");
	restore_rootcon();

	// Unmount pre-init patches
	if (access(ROOTMNT, F_OK) == 0) {
		file_readline(true, ROOTMNT, [](auto line) -> bool {
			umount2(line.data(), MNT_DETACH);
			return true;
		});
	}

	LOGI(SHOW_VER(Magisk) " daemon started\n");

	// Get server stat
	stat("/proc/self/exe", &self_st);

	// Get API level
	parse_prop_file("/system/build.prop", [](auto key, auto val) -> bool {
		if (key == "ro.build.version.sdk") {
			LOGI("* Device API level: %s\n", val.data());
			SDK_INT = parse_int(val);
			return false;
		}
		return true;
	});
	if (SDK_INT < 0) {
		// In case some devices do not store this info in build.prop, fallback to getprop
		auto sdk = getprop("ro.build.version.sdk");
		if (!sdk.empty()) {
			LOGI("* Device API level: %s\n", sdk.data());
			SDK_INT = parse_int(sdk);
		}
	}

	// Load config status
	parse_prop_file(MAGISKTMP "/config", [](auto key, auto val) -> bool {
		if (key == "RECOVERYMODE" && val == "true")
			RECOVERY_MODE = true;
		return true;
	});

	struct sockaddr_un sun;
	socklen_t len = setup_sockaddr(&sun, MAIN_SOCKET);
	fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (xbind(fd, (struct sockaddr*) &sun, len))
		exit(1);
	xlisten(fd, 10);

	// Change process name
	set_nice_name("magiskd");

	// Block all signals
	sigset_t block_set;
	sigfillset(&block_set);
	pthread_sigmask(SIG_SETMASK, &block_set, nullptr);

	// Loop forever to listen for requests
	for (;;) {
		int client = xaccept4(fd, nullptr, nullptr, SOCK_CLOEXEC);
		new_daemon_thread(request_handler, reinterpret_cast<void*>(client));
	}
}

int connect_daemon(bool create) {
	struct sockaddr_un sun;
	socklen_t len = setup_sockaddr(&sun, MAIN_SOCKET);
	int fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (connect(fd, (struct sockaddr*) &sun, len)) {
		if (!create || getuid() != UID_ROOT || getgid() != UID_ROOT) {
			LOGE("No daemon is currently running!\n");
			exit(1);
		}

		int ppid = getpid();
		LOGD("client: launching new main daemon process\n");
		if (fork_dont_care() == 0) {
			close(fd);

			// Make sure ppid is not in acct
			char src[64], dest[64];
			sprintf(src, "/acct/uid_0/pid_%d", ppid);
			sprintf(dest, "/acct/uid_0/pid_%d", getpid());
			rename(src, dest);

			main_daemon();
		}

		while (connect(fd, (struct sockaddr*) &sun, len))
			usleep(10000);
	}
	return fd;
}
