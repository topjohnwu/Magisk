#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <fcntl.h>
#include <vector>

#include <utils.hpp>

#include "init.hpp"

using namespace std;

vector<string> mount_list;

template<char... cs> using chars = integer_sequence<char, cs...>;

// If quoted, parsing ends when we find char in [breaks]
// If not quoted, parsing ends when we find char in [breaks] + [escapes]
template<char... escapes, char... breaks>
static string extract_quoted_str_until(chars<escapes...>, chars<breaks...>,
        string_view str, size_t &pos, bool &quoted) {
    string result;
    char match_array[] = {escapes..., breaks..., '"'};
    string_view match(match_array, std::size(match_array));
    for (size_t cur = pos;; ++cur) {
        cur = str.find_first_of(match, cur);
        if (cur == string_view::npos ||
            ((str[cur] == breaks) || ...) ||
            (!quoted && ((str[cur] == escapes) || ...))) {
            result.append(str.substr(pos, cur - pos));
            pos = cur;
            return result;
        }
        if (str[cur] == '"') {
            quoted = !quoted;
            result.append(str.substr(pos, cur - pos));
            pos = cur + 1;
        }
    }
}

// Parse string into key value pairs.
// The string format: [delim][key][padding]=[padding][value][delim]
template<char delim, char... padding>
static kv_pairs parse_impl(chars<padding...>, string_view str) {
    kv_pairs kv;
    char skip_array[] = {'=', padding...};
    string_view skip(skip_array, std::size(skip_array));
    bool quoted = false;
    for (size_t pos = 0u; pos < str.size(); pos = str.find_first_not_of(delim, pos)) {
        auto key = extract_quoted_str_until(
                chars<padding..., delim>{}, chars<'='>{}, str, pos, quoted);
        pos = str.find_first_not_of(skip, pos);
        if (pos == string_view::npos || str[pos] == delim) {
            kv.emplace_back(key, "");
            continue;
        }
        auto value = extract_quoted_str_until(chars<delim>{}, chars<>{}, str, pos, quoted);
        kv.emplace_back(key, value);
    }
    return kv;
}

static kv_pairs parse_cmdline(string_view str) {
    return parse_impl<' '>(chars<>{}, str);
}
static kv_pairs parse_bootconfig(string_view str) {
    return parse_impl<'\n'>(chars<' '>{}, str);
}

#define test_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))

static bool check_key_combo() {
    uint8_t bitmask[(KEY_MAX + 1) / 8];
    vector<int> events;
    constexpr const char *name = "/event";

    for (int minor = 64; minor < 96; ++minor) {
        if (xmknod(name, S_IFCHR | 0444, makedev(13, minor)))
            continue;
        int fd = open(name, O_RDONLY | O_CLOEXEC);
        unlink(name);
        if (fd < 0)
            continue;
        memset(bitmask, 0, sizeof(bitmask));
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bitmask)), bitmask);
        if (test_bit(KEY_VOLUMEUP, bitmask))
            events.push_back(fd);
        else
            close(fd);
    }
    if (events.empty())
        return false;

    run_finally fin([&] { for_each(events.begin(), events.end(), close); });

    // Return true if volume up key is held for more than 3 seconds
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

static FILE *kmsg;
static char kmsg_buf[4096];
static int vprintk(const char *fmt, va_list ap) {
    vsnprintf(kmsg_buf + 12, sizeof(kmsg_buf) - 12, fmt, ap);
    return fprintf(kmsg, "%s", kmsg_buf);
}
void setup_klog() {
    // Shut down first 3 fds
    int fd;
    if (access("/dev/null", W_OK) == 0) {
        fd = xopen("/dev/null", O_RDWR | O_CLOEXEC);
    } else {
        mknod("/null", S_IFCHR | 0666, makedev(1, 3));
        fd = xopen("/null", O_RDWR | O_CLOEXEC);
        unlink("/null");
    }
    xdup3(fd, STDIN_FILENO, O_CLOEXEC);
    xdup3(fd, STDOUT_FILENO, O_CLOEXEC);
    xdup3(fd, STDERR_FILENO, O_CLOEXEC);
    if (fd > STDERR_FILENO)
        close(fd);

    if (access("/dev/kmsg", W_OK) == 0) {
        fd = xopen("/dev/kmsg", O_WRONLY | O_CLOEXEC);
    } else {
        mknod("/kmsg", S_IFCHR | 0666, makedev(1, 11));
        fd = xopen("/kmsg", O_WRONLY | O_CLOEXEC);
        unlink("/kmsg");
    }

    kmsg = fdopen(fd, "w");
    setbuf(kmsg, nullptr);
    log_cb.d = log_cb.i = log_cb.w = log_cb.e = vprintk;
    log_cb.ex = nop_ex;
    strcpy(kmsg_buf, "magiskinit: ");

    // Disable kmsg rate limiting
    if (FILE *rate = fopen("/proc/sys/kernel/printk_devkmsg", "w")) {
        fprintf(rate, "on\n");
        fclose(rate);
    }
}

void BootConfig::set(const kv_pairs &kv) {
    for (const auto &[key, value] : kv) {
        if (key == "androidboot.slot_suffix") {
            strlcpy(slot, value.data(), sizeof(slot));
        } else if (key == "androidboot.slot") {
            slot[0] = '_';
            strlcpy(slot + 1, value.data(), sizeof(slot) - 1);
        } else if (key == "skip_initramfs") {
            skip_initramfs = true;
        } else if (key == "androidboot.force_normal_boot") {
            force_normal_boot = !value.empty() && value[0] == '1';
        } else if (key == "rootwait") {
            rootwait = true;
        } else if (key == "androidboot.android_dt_dir") {
            strlcpy(dt_dir, value.data(), sizeof(dt_dir));
        } else if (key == "androidboot.hardware") {
            strlcpy(hardware, value.data(), sizeof(hardware));
        } else if (key == "androidboot.hardware.platform") {
            strlcpy(hardware_plat, value.data(), sizeof(hardware_plat));
        } else if (key == "androidboot.fstab_suffix") {
            strlcpy(fstab_suffix, value.data(), sizeof(fstab_suffix));
        } else if (key == "qemu") {
            emulator = true;
        }
    }
}

void BootConfig::print() {
    LOGD("skip_initramfs=[%d]\n", skip_initramfs);
    LOGD("force_normal_boot=[%d]\n", force_normal_boot);
    LOGD("rootwait=[%d]\n", rootwait);
    LOGD("slot=[%s]\n", slot);
    LOGD("dt_dir=[%s]\n", dt_dir);
    LOGD("fstab_suffix=[%s]\n", fstab_suffix);
    LOGD("hardware=[%s]\n", hardware);
    LOGD("hardware.platform=[%s]\n", hardware_plat);
    LOGD("emulator=[%d]\n", emulator);
}

#define read_dt(name, key)                                          \
snprintf(file_name, sizeof(file_name), "%s/" name, config->dt_dir); \
if (access(file_name, R_OK) == 0) {                                 \
    string data = full_read(file_name);                             \
    if (!data.empty()) {                                            \
        data.pop_back();                                            \
        strlcpy(config->key, data.data(), sizeof(config->key));     \
    }                                                               \
}

void load_kernel_info(BootConfig *config) {
    // Get kernel data using procfs and sysfs
    xmkdir("/proc", 0755);
    xmount("proc", "/proc", "proc", 0, nullptr);
    xmkdir("/sys", 0755);
    xmount("sysfs", "/sys", "sysfs", 0, nullptr);

    mount_list.emplace_back("/proc");
    mount_list.emplace_back("/sys");

    // Log to kernel
    setup_klog();

    config->set(parse_cmdline(full_read("/proc/cmdline")));
    config->set(parse_bootconfig(full_read("/proc/bootconfig")));

    parse_prop_file("/.backup/.magisk", [=](auto key, auto value) -> bool {
        if (key == "RECOVERYMODE" && value == "true") {
            LOGD("Running in recovery mode, waiting for key...\n");
            config->skip_initramfs = config->emulator || !check_key_combo();
            return false;
        }
        return true;
    });

    if (config->dt_dir[0] == '\0')
        strlcpy(config->dt_dir, DEFAULT_DT_DIR, sizeof(config->dt_dir));

    char file_name[128];
    read_dt("fstab_suffix", fstab_suffix)
    read_dt("hardware", hardware)
    read_dt("hardware.platform", hardware_plat)

    LOGD("Device config:\n");
    config->print();
}

bool check_two_stage() {
    if (access("/apex", F_OK) == 0)
        return true;
    if (access("/system/bin/init", F_OK) == 0)
        return true;
    // If we still have no indication, parse the original init and see what's up
    auto init = mmap_data(backup_init());
    return init.contains("selinux_setup");
}

const char *backup_init() {
    if (access("/.backup/init.real", F_OK) == 0)
        return "/.backup/init.real";
    return "/.backup/init";
}
