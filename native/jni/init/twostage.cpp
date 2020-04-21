#include <magisk.hpp>
#include <utils.hpp>
#include <logging.hpp>
#include <socket.hpp>

#include "init.hpp"

using namespace std;

static void patch_fstab(const char *fstab) {
	string patched = fstab + ".p"s;
	FILE *fp = xfopen(patched.data(), "we");
	file_readline(fstab, [=](string_view l) -> bool {
		if (l[0] == '#' || l.length() == 1)
			return true;
		char *line = (char *) l.data();
		int src0, src1, mnt0, mnt1, type0, type1, opt0, opt1, flag0, flag1;
		sscanf(line, "%n%*s%n %n%*s%n %n%*s%n %n%*s%n %n%*s%n",
			   &src0, &src1, &mnt0, &mnt1, &type0, &type1, &opt0, &opt1, &flag0, &flag1);
		const char *src, *mnt, *type, *opt, *flag;
		src = &line[src0];
		line[src1] = '\0';
		mnt = &line[mnt0];
		line[mnt1] = '\0';
		type = &line[type0];
		line[type1] = '\0';
		opt = &line[opt0];
		line[opt1] = '\0';
		flag = &line[flag0];
		line[flag1] = '\0';

		// Redirect system to system_root
		if (mnt == "/system"sv)
			mnt = "/system_root";

		fprintf(fp, "%s %s %s %s %s\n", src, mnt, type, opt, flag);
		return true;
	});
	fclose(fp);

	// Replace old fstab
	clone_attr(fstab, patched.data());
	rename(patched.data(), fstab);
}

#define FSR "/first_stage_ramdisk"

void FirstStageInit::prepare() {
	if (cmd->force_normal_boot) {
		xmkdirs(FSR "/system/bin", 0755);
		rename("/init" /* magiskinit */, FSR "/system/bin/init");
		symlink("/system/bin/init", FSR "/init");
		rename("/.backup", FSR "/.backup");
		rename("/overlay.d", FSR "/overlay.d");
		xsymlink("/system/bin/init", "/init");

		chdir(FSR);
	} else {
		xmkdir("/system", 0755);
		xmkdir("/system/bin", 0755);
		rename("/init" /* magiskinit */ , "/system/bin/init");
		rename("/.backup/init", "/init");
	}

	// Patch fstab
	auto dir = xopen_dir(".");
	for (dirent *de; (de = xreaddir(dir.get()));) {
		if (strstr(de->d_name, "fstab")) {
			patch_fstab(de->d_name);
			break;
		}
	}
	chdir("/");
}

#define INIT_PATH  "/system/bin/init"
#define REDIR_PATH "/system/bin/am"

void SARFirstStageInit::prepare() {
	int pid = getpid();

	xmount("tmpfs", "/dev", "tmpfs", 0, "mode=755");

	// Patch init binary
	raw_data init;
	int src = xopen("/init", O_RDONLY);
	fd_full_read(src, init.buf, init.sz);
	for (uint8_t *p = init.buf, *eof = init.buf + init.sz; p < eof; ++p) {
		if (memcmp(p, INIT_PATH, sizeof(INIT_PATH)) == 0) {
			LOGD("Patch init [" INIT_PATH "] -> [" REDIR_PATH "]\n");
			memcpy(p, REDIR_PATH, sizeof(REDIR_PATH));
			break;
		}
	}
	int dest = xopen("/dev/init", O_CREAT | O_WRONLY, 0);
	write(dest, init.buf, init.sz);
	fclone_attr(src, dest);
	close(dest);

	// Replace redirect init with magiskinit
	dest = xopen("/dev/magiskinit", O_CREAT | O_WRONLY, 0);
	write(dest, self.buf, self.sz);
	fclone_attr(src, dest);
	close(src);
	close(dest);

	xmount("/dev/init", "/init", nullptr, MS_BIND, nullptr);
	xmount("/dev/magiskinit", REDIR_PATH, nullptr, MS_BIND, nullptr);
	xumount2("/dev", MNT_DETACH);

	// Block SIGUSR1
	sigset_t block, old;
	sigemptyset(&block);
	sigaddset(&block, SIGUSR1);
	sigprocmask(SIG_BLOCK, &block, &old);

	if (int child = xfork(); child) {
		LOGD("init daemon [%d]\n", child);
		// Wait for children signal
		int sig;
		sigwait(&block, &sig);

		// Restore sigmask
		sigprocmask(SIG_SETMASK, &old, nullptr);
	} else {
		// Establish socket for 2nd stage ack
		struct sockaddr_un sun;
		int sockfd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
		xbind(sockfd, (struct sockaddr*) &sun, setup_sockaddr(&sun, INIT_SOCKET));
		xlisten(sockfd, 1);

		// Resume parent
		kill(pid, SIGUSR1);

		// Wait for second stage ack
		int client = xaccept4(sockfd, nullptr, nullptr, SOCK_CLOEXEC);

		// Write backup files
		char *tmp_dir = read_string(client);
		chdir(tmp_dir);
		free(tmp_dir);
		int cfg = xopen(INTLROOT "/config", O_WRONLY | O_CREAT, 0);
		xwrite(cfg, config.buf, config.sz);
		close(cfg);
		restore_folder(ROOTOVL, overlays);

		// Ack and bail out!
		write(sockfd, &sockfd, sizeof(sockfd));
		close(client);
		close(sockfd);

		exit(0);
	}
}
