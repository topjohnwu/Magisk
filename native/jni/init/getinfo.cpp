#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <fcntl.h>
#include <vector>

#include <utils.h>
#include <logging.h>

#include "init.h"

using namespace std;

#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"

static void parse_cmdline(const std::function<void (std::string_view, const char *)> &fn) {
	char cmdline[4096];
	int fd = open("/proc/cmdline", O_RDONLY | O_CLOEXEC);
	cmdline[read(fd, cmdline, sizeof(cmdline))] = '\0';
	close(fd);

	char *tok, *eql, *tmp, *saveptr;
	saveptr = cmdline;
	while ((tok = strtok_r(nullptr, " \n", &saveptr)) != nullptr) {
		eql = strchr(tok, '=');
		if (eql) {
			*eql = '\0';
			if (eql[1] == '"') {
				tmp = strchr(saveptr, '"');
				if (tmp != nullptr) {
					*tmp = '\0';
					saveptr[-1] = ' ';
					saveptr = tmp + 1;
					eql++;
				}
			}
			fn(tok, eql + 1);
		} else {
			fn(tok, "");
		}
	}
}

#define test_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))

static bool check_key_combo() {
	uint8_t bitmask[(KEY_MAX + 1) / 8];
	vector<int> events;
	constexpr const char *name = "/event";

	for (int minor = 64; minor < 96; ++minor) {
		if (mknod(name, S_IFCHR | 0444, makedev(13, minor))) {
			PLOGE("mknod");
			continue;
		}
		int fd = open(name, O_RDONLY | O_CLOEXEC);
		unlink(name);
		if (fd < 0)
			continue;
		memset(bitmask, 0, sizeof(bitmask));
		ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bitmask)), bitmask);
		if (test_bit(KEY_VOLUMEUP, bitmask))
			events.push_back(fd);
	}

	if (events.empty())
		return false;

	RunFinally fin([&]() -> void {
		for (const int &fd : events)
			close(fd);
	});

	// Return true if volume key up is hold for more than 3 seconds
	int count = 0;
	for (int i = 0; i < 500; ++i) {
		for (const int &fd : events) {
			memset(bitmask, 0, sizeof(bitmask));
			ioctl(fd, EVIOCGKEY(sizeof(bitmask)), bitmask);
			if (test_bit(KEY_VOLUMEUP, bitmask)) {
				count++;
				break;
			}
		}
		if (count >= 300) {
			LOGD("KEY_VOLUMEUP detected: disable system-as-root\n");
			return true;
		}
		// Check every 10ms
		usleep(10000);
	}
	return false;
}

void load_kernel_info(cmdline *cmd) {
	// Communicate with kernel using procfs and sysfs
	xmkdir("/proc", 0755);
	xmount("proc", "/proc", "proc", 0, nullptr);
	xmkdir("/sys", 0755);
	xmount("sysfs", "/sys", "sysfs", 0, nullptr);

	bool enter_recovery = false;
	bool kirin = false;
	bool recovery_mode = false;

	parse_cmdline([&](auto key, auto value) -> void {
		LOGD("cmdline: [%s]=[%s]\n", key.data(), value);
		if (key == "androidboot.slot_suffix") {
			strcpy(cmd->slot, value);
		} else if (key == "androidboot.slot") {
			cmd->slot[0] = '_';
			strcpy(cmd->slot + 1, value);
		} else if (key == "skip_initramfs") {
			cmd->system_as_root = true;
		} else if (key == "androidboot.force_normal_boot") {
			cmd->force_normal_boot = value[0] == '1';
		} else if (key == "androidboot.android_dt_dir") {
			strcpy(cmd->dt_dir, value);
		} else if (key == "enter_recovery") {
			enter_recovery = value[0] == '1';
		} else if (key == "androidboot.hardware") {
			kirin = strstr(value, "kirin") || strstr(value, "hi3660") || strstr(value, "hi6250");
		}
	});

	parse_prop_file("/.backup/.magisk", [&](auto key, auto value) -> bool {
		if (key == "RECOVERYMODE" && value == "true")
			recovery_mode = true;
		return true;
	});

	if (kirin && enter_recovery) {
		// Inform that we are actually booting as recovery
		if (!recovery_mode) {
			if (FILE *f = fopen("/.backup/.magisk", "ae"); f) {
				fprintf(f, "RECOVERYMODE=true\n");
				fclose(f);
			}
			recovery_mode = true;
		}
	}

	if (recovery_mode) {
		LOGD("Running in recovery mode, waiting for key...\n");
		cmd->system_as_root = !check_key_combo();
	}

	if (cmd->dt_dir[0] == '\0')
		strcpy(cmd->dt_dir, DEFAULT_DT_DIR);

	LOGD("system_as_root=[%d]\n", cmd->system_as_root);
	LOGD("force_normal_boot=[%d]\n", cmd->force_normal_boot);
	LOGD("slot=[%s]\n", cmd->slot);
	LOGD("dt_dir=[%s]\n", cmd->dt_dir);
}
