#include <sys/ptrace.h>
#include <sys/wait.h>

#include <utils.hpp>
#include <logging.hpp>

#include "init.hpp"

using namespace std;

static void patch_fstab(const string &fstab) {
	string patched = fstab + ".p";
	FILE *fp = xfopen(patched.data(), "we");
	file_readline(fstab.data(), [=](string_view l) -> bool {
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
	clone_attr(fstab.data(), patched.data());
	rename(patched.data(), fstab.data());
}

#define FSR "/first_stage_ramdisk"

void ForcedFirstStageInit::prepare() {
	// It is actually possible to NOT have FSR, create it just in case
	xmkdir(FSR, 0755);

	if (auto dir = xopen_dir(FSR); dir) {
		string fstab(FSR "/");
		for (dirent *de; (de = xreaddir(dir.get()));) {
			if (strstr(de->d_name, "fstab")) {
				fstab += de->d_name;
				break;
			}
		}
		if (fstab.length() == sizeof(FSR))
			return;

		patch_fstab(fstab);
	} else {
		return;
	}

	// Move stuffs for next stage
	xmkdir(FSR "/system", 0755);
	xmkdir(FSR "/system/bin", 0755);
	rename("/init", FSR "/system/bin/init");
	symlink("/system/bin/init", FSR "/init");
	xmkdir(FSR "/.backup", 0);
	rename("/.backup/.magisk", FSR "/.backup/.magisk");
	rename("/overlay.d", FSR "/overlay.d");
}

void FirstStageInit::prepare() {
	auto dir = xopen_dir("/");
	for (dirent *de; (de = xreaddir(dir.get()));) {
		if (strstr(de->d_name, "fstab")) {
			patch_fstab(de->d_name);
			break;
		}
	}

	// Move stuffs for next stage
	xmkdir("/system", 0755);
	xmkdir("/system/bin", 0755);
	rename("/init", "/system/bin/init");
	rename("/.backup/init", "/init");
}

static inline long xptrace(int request, pid_t pid, void *addr, void *data) {
	long ret = ptrace(request, pid, addr, data);
	if (ret < 0)
		PLOGE("ptrace %d", pid);
	return ret;
}

static inline long xptrace(int request, pid_t pid, void *addr = nullptr, intptr_t data = 0) {
	return xptrace(request, pid, addr, reinterpret_cast<void *>(data));
}

#define INIT_SOCKET "MAGISKINIT"

socklen_t setup_sockaddr(struct sockaddr_un *sun) {
	sun->sun_family = AF_LOCAL;
	strcpy(sun->sun_path + 1, INIT_SOCKET);
	return sizeof(sa_family_t) + sizeof(INIT_SOCKET);
}

void SARFirstStageInit::traced_exec_init() {
	int pid = getpid();

	// Block SIGUSR1
	sigset_t block, old;
	sigemptyset(&block);
	sigaddset(&block, SIGUSR1);
	sigprocmask(SIG_BLOCK, &block, &old);

	if (int child = xfork(); child) {
		LOGD("init tracer [%d]\n", child);
		// Wait for children to attach
		int sig;
		sigwait(&block, &sig);

		// Restore sigmask
		sigprocmask(SIG_BLOCK, &old, nullptr);

		// Re-exec init
		exec_init();
	} else {
		// Close all file descriptors and stop logging
		no_logging();
		for (int i = 0; i < 20; ++i)
			close(i);

		// Attach to parent to trace exec
		xptrace(PTRACE_ATTACH, pid);
		waitpid(pid, nullptr, __WALL | __WNOTHREAD);
		xptrace(PTRACE_SETOPTIONS, pid, nullptr, PTRACE_O_TRACEEXEC);
		xptrace(PTRACE_CONT, pid, 0, SIGUSR1);

		// Wait for execve
		waitpid(pid, nullptr, __WALL | __WNOTHREAD);

		// Swap out init with bind mount
		xmount("tmpfs", "/dev", "tmpfs", 0, "mode=755");
		int init = xopen("/dev/magisk", O_CREAT | O_WRONLY, 0750);
		write(init, self.buf, self.sz);
		close(init);
		xmount("/dev/magisk", "/init", nullptr, MS_BIND, nullptr);
		xumount2("/dev", MNT_DETACH);

		xptrace(PTRACE_DETACH, pid);

		// Start daemon for 2nd stage preparation
		struct sockaddr_un sun{};
		auto len = setup_sockaddr(&sun);
		int sockfd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
		xbind(sockfd, (struct sockaddr*) &sun, len);
		xlisten(sockfd, 1);

		// Wait for second stage ack
		int client = xaccept4(sockfd, nullptr, nullptr, SOCK_CLOEXEC);

		// Write backup files
		int cfg = xopen(MAGISKTMP "/config", O_WRONLY | O_CREAT, 0000);
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
