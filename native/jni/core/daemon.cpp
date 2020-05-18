#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <libgen.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <android/log.h>

#include <magisk.hpp>
#include <utils.hpp>
#include <daemon.hpp>
#include <selinux.hpp>
#include <db.hpp>
#include <resetprop.hpp>
#include <flags.h>

using namespace std;

int SDK_INT = -1;
bool RECOVERY_MODE = false;
string MAGISKTMP;
int DAEMON_STATE = STATE_UNKNOWN;

static struct stat self_st;

static bool verify_client(int client, pid_t pid) {
	// Verify caller is the same as server
	char path[32];
	sprintf(path, "/proc/%d/exe", pid);
	struct stat st;
	return !(stat(path, &st) || st.st_dev != self_st.st_dev || st.st_ino != self_st.st_ino);
}

static void request_handler(int client, int req_code, ucred cred) {
	switch (req_code) {
	case MAGISKHIDE:
		magiskhide_handler(client);
		break;
	case SUPERUSER:
		su_daemon_handler(client, &cred);
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
		foreach_modules("remove");
		write_int(client, 0);
		close(client);
		reboot();
		break;
	default:
		close(client);
		break;
	}
}

static void handle_request(int client) {
	int req_code;

	// Verify client credentials
	ucred cred;
	get_client_cred(client, &cred);
	if (cred.uid != 0 && !verify_client(client, cred.pid))
		goto shortcut;

	// Check client permissions
	req_code = read_int(client);
	switch (req_code) {
	case MAGISKHIDE:
	case POST_FS_DATA:
	case LATE_START:
	case BOOT_COMPLETE:
	case SQLITE_CMD:
	case GET_PATH:
		if (cred.uid != 0) {
			write_int(client, ROOT_REQUIRED);
			goto shortcut;
		}
		break;
	case REMOVE_MODULES:
		if (cred.uid != UID_SHELL && cred.uid != UID_ROOT) {
			write_int(client, 1);
			goto shortcut;
		}
		break;
	}

	switch (req_code) {
	// In case of init trigger launches, set the corresponding states
	case POST_FS_DATA:
		DAEMON_STATE = STATE_POST_FS_DATA;
		break;
	case LATE_START:
		DAEMON_STATE = STATE_LATE_START;
		break;
	case BOOT_COMPLETE:
		DAEMON_STATE = STATE_BOOT_COMPLETE;
		break;

	// Simple requests to query daemon info
	case CHECK_VERSION:
		write_string(client, MAGISK_VERSION ":MAGISK");
		goto shortcut;
	case CHECK_VERSION_CODE:
		write_int(client, MAGISK_VER_CODE);
		goto shortcut;
	case GET_PATH:
		write_string(client, MAGISKTMP.data());
		goto shortcut;
	case DO_NOTHING:
		goto shortcut;
	}

	// Create new thread to handle complex requests
	new_daemon_thread(std::bind(&request_handler, client, req_code, cred));
	return;

shortcut:
	close(client);
}

static void android_logging() {
	static constexpr char TAG[] = "Magisk";
#ifdef MAGISK_DEBUG
	log_cb.d = [](auto fmt, auto ap){ return __android_log_vprint(ANDROID_LOG_DEBUG, TAG, fmt, ap); };
#else
	log_cb.d = nop_log;
#endif
	log_cb.i = [](auto fmt, auto ap){ return __android_log_vprint(ANDROID_LOG_INFO, TAG, fmt, ap); };
	log_cb.w = [](auto fmt, auto ap){ return __android_log_vprint(ANDROID_LOG_WARN, TAG, fmt, ap); };
	log_cb.e = [](auto fmt, auto ap){ return __android_log_vprint(ANDROID_LOG_ERROR, TAG, fmt, ap); };
	log_cb.ex = nop_ex;
}

static void daemon_entry(int ppid) {
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

	setsid();
	setcon("u:r:" SEPOL_PROC_DOMAIN ":s0");

	// Make sure ppid is not in acct
	char src[64], dest[64];
	sprintf(src, "/acct/uid_0/pid_%d", ppid);
	sprintf(dest, "/acct/uid_0/pid_%d", getpid());
	rename(src, dest);

	// Get self stat
	xreadlink("/proc/self/exe", src, sizeof(src));
	MAGISKTMP = dirname(src);
	xstat("/proc/self/exe", &self_st);

	restore_tmpcon();

	// SAR cleanups
	auto mount_list = MAGISKTMP + "/" ROOTMNT;
	if (access(mount_list.data(), F_OK) == 0) {
		file_readline(true, mount_list.data(), [](string_view line) -> bool {
			umount2(line.data(), MNT_DETACH);
			return true;
		});
	}
	unlink("/dev/.se");

	LOGI(NAME_WITH_VER(Magisk) " daemon started\n");

	// Get API level
	parse_prop_file("/system/build.prop", [](auto key, auto val) -> bool {
		if (key == "ro.build.version.sdk") {
			SDK_INT = parse_int(val);
			return false;
		}
		return true;
	});
	if (SDK_INT < 0) {
		// In case some devices do not store this info in build.prop, fallback to getprop
		auto sdk = getprop("ro.build.version.sdk");
		if (!sdk.empty()) {
			SDK_INT = parse_int(sdk);
		}
	}
	LOGI("* Device API level: %d\n", SDK_INT);

	// Load config status
	auto config = MAGISKTMP + "/" INTLROOT "/config";
	parse_prop_file(config.data(), [](auto key, auto val) -> bool {
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
		handle_request(client);
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
			daemon_entry(ppid);
		}

		while (connect(fd, (struct sockaddr*) &sun, len))
			usleep(10000);
	}
	return fd;
}
