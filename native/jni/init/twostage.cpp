#include <magisk.hpp>
#include <utils.hpp>
#include <logging.hpp>
#include <socket.hpp>

#include "init.hpp"

using namespace std;

void fstab_entry::to_file(FILE *fp) {
	fprintf(fp, "%s %s %s %s %s\n", dev.data(), mnt_point.data(),
			type.data(), mnt_flags.data(), fsmgr_flags.data());
}

#define set_info(val) \
line[val##1] = '\0'; \
entry.val = &line[val##0];

#define FSR "/first_stage_ramdisk"

extern uint32_t patch_verity(void *buf, uint32_t size);

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

	// Try to load fstab from dt
	vector<fstab_entry> fstab;
	read_dt_fstab(fstab);

	char fstab_file[128];
	fstab_file[0] = '\0';

	// Find existing fstab file
	for (const char *hw : { cmd->hardware, cmd->hardware_plat }) {
		if (hw[0] == '\0')
			continue;
		sprintf(fstab_file, "fstab.%s", hw);
		if (access(fstab_file, F_OK) != 0) {
			fstab_file[0] = '\0';
			continue;
		} else {
			LOGD("Found fstab file: %s\n", fstab_file);
			break;
		}
	}

	if (fstab.empty()) {
		// fstab has to be somewhere in ramdisk
		if (fstab_file[0] == '\0') {
			LOGE("Cannot find fstab file in ramdisk!\n");
			return;
		}

		// Parse and load fstab file
		file_readline(fstab_file, [&](string_view l) -> bool {
			if (l[0] == '#' || l.length() == 1)
				return true;
			char *line = (char *) l.data();

			int dev0, dev1, mnt_point0, mnt_point1, type0, type1,
					mnt_flags0, mnt_flags1, fsmgr_flags0, fsmgr_flags1;

			sscanf(line, "%n%*s%n %n%*s%n %n%*s%n %n%*s%n %n%*s%n",
				   &dev0, &dev1, &mnt_point0, &mnt_point1, &type0, &type1,
				   &mnt_flags0, &mnt_flags1, &fsmgr_flags0, &fsmgr_flags1);

			fstab_entry entry;

			set_info(dev);
			set_info(mnt_point);
			set_info(type);
			set_info(mnt_flags);
			set_info(fsmgr_flags);

			fstab.emplace_back(std::move(entry));
			return true;
		});
	} else {
		// All dt fstab entries should be first_stage_mount
		for (auto &entry : fstab) {
			if (!str_contains(entry.fsmgr_flags, "first_stage_mount")) {
				if (!entry.fsmgr_flags.empty())
					entry.fsmgr_flags += ',';
				entry.fsmgr_flags += "first_stage_mount";
			}
		}

		if (fstab_file[0] == '\0') {
			const char *hw = cmd->hardware[0] ? cmd->hardware :
					(cmd->hardware_plat[0] ? cmd->hardware_plat : nullptr);
			if (hw == nullptr) {
				LOGE("Cannot determine hardware name!\n");
				return;
			}
			sprintf(fstab_file, "fstab.%s", hw);
		}

		// Patch init to force ignore dt fstab
		uint8_t *addr;
		size_t sz;
		mmap_rw("/init", addr, sz);
		raw_data_patch(addr, sz, {
			make_pair("android,fstab", "xxx")  /* Force IsDtFstabCompatible() to return false */
		});
		munmap(addr, sz);
	}

	{
		LOGD("Write fstab file: %s\n", fstab_file);
		auto fp = xopen_file(fstab_file, "we");
		for (auto &entry : fstab) {
			// Redirect system mnt_point so init won't switch root in first stage init
			if (entry.mnt_point == "/system")
				entry.mnt_point = "/system_root";

			// Force remove AVB for 2SI since it may bootloop some devices
			auto len = patch_verity(entry.fsmgr_flags.data(), entry.fsmgr_flags.length());
			entry.fsmgr_flags.resize(len);

			entry.to_file(fp.get());
		}
	}
	chmod(fstab_file, 0644);

	chdir("/");
}

#define INIT_PATH  "/system/bin/init"
#define REDIR_PATH "/system/bin/am"

void SARFirstStageInit::prepare() {
	int pid = getpid();

	xmount("tmpfs", "/dev", "tmpfs", 0, "mode=755");

	// Patch init binary
	int src = xopen("/init", O_RDONLY);
	int dest = xopen("/dev/init", O_CREAT | O_WRONLY, 0);
	{
		raw_data init;
		fd_full_read(src, init.buf, init.sz);
		raw_data_patch(init.buf, init.sz, { make_pair(INIT_PATH, REDIR_PATH) });
		write(dest, init.buf, init.sz);
		fclone_attr(src, dest);
		close(dest);
	}

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
