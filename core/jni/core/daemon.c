/* daemon.c - Magisk Daemon
 *
 * Start the daemon and wait for requests
 * Connect the daemon and send requests through sockets
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <selinux/selinux.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"
#include "resetprop.h"

int is_daemon_init = 0, seperate_vendor = 0;

static void *request_handler(void *args) {
	int client = *((int *) args);
	free(args);
	client_request req = read_int(client);

	struct ucred credential;
	get_client_cred(client, &credential);

	switch (req) {
	case LAUNCH_MAGISKHIDE:
	case STOP_MAGISKHIDE:
	case ADD_HIDELIST:
	case RM_HIDELIST:
	case LS_HIDELIST:
	case POST_FS:
	case POST_FS_DATA:
	case LATE_START:
		if (credential.uid != 0) {
			write_int(client, ROOT_REQUIRED);
			close(client);
			return NULL;
		}
	default:
		break;
	}

	switch (req) {
	case LAUNCH_MAGISKHIDE:
		launch_magiskhide(client);
		break;
	case STOP_MAGISKHIDE:
		stop_magiskhide(client);
		break;
	case ADD_HIDELIST:
		add_hide_list(client);
		break;
	case RM_HIDELIST:
		rm_hide_list(client);
		break;
	case LS_HIDELIST:
		ls_hide_list(client);
		break;
	case SUPERUSER:
		su_daemon_receiver(client, &credential);
		break;
	case CHECK_VERSION:
		write_string(client, MAGISK_VER_STR);
		close(client);
		break;
	case CHECK_VERSION_CODE:
		write_int(client, MAGISK_VER_CODE);
		close(client);
		break;
	case POST_FS:
		post_fs(client);
		break;
	case POST_FS_DATA:
		post_fs_data(client);
		break;
	case LATE_START:
		late_start(client);
		break;
	default:
		break;
	}
	return NULL;
}


static void *start_magisk_hide(void *args) {
	launch_magiskhide(-1);
	return NULL;
}

void auto_start_magiskhide() {
	char *hide_prop = getprop2(MAGISKHIDE_PROP, 1);
	if (hide_prop == NULL || strcmp(hide_prop, "0") != 0) {
		pthread_t thread;
		xpthread_create(&thread, NULL, start_magisk_hide, NULL);
		pthread_detach(thread);
	}
	free(hide_prop);
}

void daemon_init() {
	is_daemon_init = 1;

	 // Magisk binaries
	char *bin_path = NULL;
	if (access("/cache/data_bin", F_OK) == 0)
		bin_path = "/cache/data_bin";
	else if (access("/data/data/com.topjohnwu.magisk/install", F_OK) == 0)
		bin_path = "/data/data/com.topjohnwu.magisk/install";
	else if (access("/data/user_de/0/com.topjohnwu.magisk/install", F_OK) == 0)
		bin_path = "/data/user_de/0/com.topjohnwu.magisk/install";
	if (bin_path) {
		rm_rf(DATABIN);
		cp_afc(bin_path, DATABIN);
		rm_rf(bin_path);
	}

	// Migration
	rm_rf("/data/magisk");
	unlink("/data/magisk.img");
	unlink("/data/magisk_debug.log");
	chmod("/data/adb", 0700);

	// Use shell glob to match files
	exec_command_sync("sh", "-c",
			"mv -f /data/adb/magisk/stock_*.img.gz /data;"
			"rm -f /data/user*/*/magisk.db;", NULL);

	LOGI("* Creating /sbin overlay");
	DIR *dir;
	struct dirent *entry;
	int root, sbin;
	char buf[PATH_MAX], buf2[PATH_MAX];

	// Setup links under /sbin
	xmount(NULL, "/", NULL, MS_REMOUNT, NULL);
	xmkdir("/root", 0755);
	chmod("/root", 0755);
	root = xopen("/root", O_RDONLY | O_CLOEXEC);
	sbin = xopen("/sbin", O_RDONLY | O_CLOEXEC);
	dir = xfdopendir(sbin);
	while((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		linkat(sbin, entry->d_name, root, entry->d_name, 0);
		if (strcmp(entry->d_name, "magisk") == 0)
			unlinkat(sbin, entry->d_name, 0);
	}
	close(sbin);

	xmount("tmpfs", "/sbin", "tmpfs", 0, NULL);
	chmod("/sbin", 0755);
	setfilecon("/sbin", "u:object_r:rootfs:s0");
	dir = xfdopendir(root);
	while((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		snprintf(buf, PATH_MAX, "/root/%s", entry->d_name);
		snprintf(buf2, PATH_MAX, "/sbin/%s", entry->d_name);
		xsymlink(buf, buf2);
	}
	for (int i = 0; applet[i]; ++i) {
		snprintf(buf2, PATH_MAX, "/sbin/%s", applet[i]);
		xsymlink("/root/magisk", buf2);
	}
	for (int i = 0; init_applet[i]; ++i) {
		snprintf(buf2, PATH_MAX, "/sbin/%s", init_applet[i]);
		xsymlink("/root/magiskinit", buf2);
	}
	close(root);

	// Backward compatibility
	xsymlink(DATABIN, "/data/magisk");
	xsymlink(MAINIMG, "/data/magisk.img");
	xsymlink(MOUNTPOINT, "/magisk");

	xmount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL);

	LOGI("* Mounting mirrors");
	struct vector mounts;
	vec_init(&mounts);
	file_to_vector("/proc/mounts", &mounts);
	char *line;
	int skip_initramfs = 0;
	// Check whether skip_initramfs device
	vec_for_each(&mounts, line) {
		if (strstr(line, " /system_root ")) {
			xmkdir_p(MIRRDIR "/system", 0755);
			bind_mount("/system_root/system", MIRRDIR "/system");
			skip_initramfs = 1;
			break;
		}
	}
	vec_for_each(&mounts, line) {
		if (!skip_initramfs && strstr(line, " /system ")) {
			sscanf(line, "%s", buf);
			xmkdir_p(MIRRDIR "/system", 0755);
			xmount(buf, MIRRDIR "/system", "ext4", MS_RDONLY, NULL);
			#ifdef MAGISK_DEBUG
				LOGI("mount: %s -> %s\n", buf, MIRRDIR "/system");
			#else
				LOGI("mount: %s\n", MIRRDIR "/system");
			#endif
		} else if (strstr(line, " /vendor ")) {
			seperate_vendor = 1;
			sscanf(line, "%s", buf);
			xmkdir_p(MIRRDIR "/vendor", 0755);
			xmount(buf, MIRRDIR "/vendor", "ext4", MS_RDONLY, NULL);
			#ifdef MAGISK_DEBUG
				LOGI("mount: %s -> %s\n", buf, MIRRDIR "/vendor");
			#else
				LOGI("mount: %s\n", MIRRDIR "/vendor");
			#endif
		}
		free(line);
	}
	vec_destroy(&mounts);
	if (!seperate_vendor) {
		xsymlink(MIRRDIR "/system/vendor", MIRRDIR "/vendor");
		#ifdef MAGISK_DEBUG
			LOGI("link: %s -> %s\n", MIRRDIR "/system/vendor", MIRRDIR "/vendor");
		#else
			LOGI("link: %s\n", MIRRDIR "/vendor");
		#endif
	}
	xmkdir_p(MIRRDIR "/bin", 0755);
	bind_mount(DATABIN, MIRRDIR "/bin");

	LOGI("* Setting up internal busybox");
	xmkdir_p(BBPATH, 0755);
	exec_command_sync(MIRRDIR "/bin/busybox", "--install", "-s", BBPATH, NULL);
	xsymlink(MIRRDIR "/bin/busybox", BBPATH "/busybox");
}

void start_daemon() {
	setsid();
	setcon("u:r:su:s0");
	umask(0);
	int fd = xopen("/dev/null", O_RDWR | O_CLOEXEC);
	xdup2(fd, STDIN_FILENO);
	xdup2(fd, STDOUT_FILENO);
	xdup2(fd, STDERR_FILENO);
	close(fd);

	// Block user signals
	sigset_t block_set;
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGUSR1);
	sigaddset(&block_set, SIGUSR2);
	pthread_sigmask(SIG_SETMASK, &block_set, NULL);

	struct sockaddr_un sun;
	fd = setup_socket(&sun);

	if (xbind(fd, (struct sockaddr*) &sun, sizeof(sun)))
		exit(1);
	xlisten(fd, 10);

	if ((is_daemon_init = (access(MAGISKTMP, F_OK) == 0))) {
		// Restart stuffs if the daemon is restarted
		exec_command_sync("logcat", "-b", "all", "-c", NULL);
		auto_start_magiskhide();
		start_debug_log();
	} else if (check_data()) {
		daemon_init();
	}

	// Start the log monitor
	monitor_logs();

	LOGI("Magisk v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") daemon started\n");

	// Change process name
	strcpy(argv0, "magisk_daemon");

	// Unlock all blocks for rw
	unlock_blocks();

	// Loop forever to listen for requests
	while(1) {
		int *client = xmalloc(sizeof(int));
		*client = xaccept4(fd, NULL, NULL, SOCK_CLOEXEC);
		pthread_t thread;
		xpthread_create(&thread, NULL, request_handler, client);
		// Detach the thread, we will never join it
		pthread_detach(thread);
	}
}

/* Connect the daemon, and return a socketfd */
int connect_daemon() {
	struct sockaddr_un sun;
	int fd = setup_socket(&sun);
	if (connect(fd, (struct sockaddr*) &sun, sizeof(sun))) {
		// If we cannot access the daemon, we start a daemon in the child process if possible

		if (getuid() != UID_ROOT || getgid() != UID_ROOT) {
			fprintf(stderr, "No daemon is currently running!\n");
			exit(1);
		}

		if (xfork() == 0) {
			LOGD("client: connect fail, try launching new daemon process\n");
			close(fd);
			start_daemon();
		}

		while (connect(fd, (struct sockaddr*) &sun, sizeof(sun)))
			usleep(10000);
	}
	return fd;
}
