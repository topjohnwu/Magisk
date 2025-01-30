#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <fcntl.h>
#include <vector>

#include <base.hpp>

#include "init.hpp"

using namespace std;

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
// The string format: [delim][key][padding][eq][padding][value][delim]
template<char delim, char eq, char... padding>
static kv_pairs parse_impl(chars<padding...>, string_view str) {
    kv_pairs kv;
    char skip_array[] = {eq, padding...};
    string_view skip(skip_array, std::size(skip_array));
    bool quoted = false;
    for (size_t pos = 0u; pos < str.size(); pos = str.find_first_not_of(delim, pos)) {
        auto key = extract_quoted_str_until(
                chars<padding..., delim>{}, chars<eq>{}, str, pos, quoted);
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
    return parse_impl<' ', '='>(chars<>{}, str);
}
static kv_pairs parse_bootconfig(string_view str) {
    return parse_impl<'\n', '='>(chars<' '>{}, str);
}
static kv_pairs parse_partition_map(std::string_view str) {
    return parse_impl<';', ','>(chars<>{}, str);
}

#define test_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))

static bool check_key_combo() {
    LOGD("Running in recovery mode, waiting for key...\n");
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

void BootConfig::set(const kv_pairs &kv) noexcept {
    for (const auto &[key, value] : kv) {
        if (key == "androidboot.slot_suffix") {
            // Many Amlogic devices are A-only but have slot_suffix...
            if (value == "normal") {
                LOGW("Skip invalid androidboot.slot_suffix=[normal]\n");
                continue;
            }
            strscpy(slot.data(), value.data(), slot.size());
        } else if (key == "androidboot.slot") {
            slot[0] = '_';
            strscpy(slot.data() + 1, value.data(), slot.size() - 1);
        } else if (key == "skip_initramfs") {
            skip_initramfs = true;
        } else if (key == "androidboot.force_normal_boot") {
            force_normal_boot = !value.empty() && value[0] == '1';
        } else if (key == "rootwait") {
            rootwait = true;
        } else if (key == "androidboot.android_dt_dir") {
            strscpy(dt_dir.data(), value.data(), dt_dir.size());
        } else if (key == "androidboot.hardware") {
            strscpy(hardware.data(), value.data(), hardware.size());
        } else if (key == "androidboot.hardware.platform") {
            strscpy(hardware_plat.data(), value.data(), hardware_plat.size());
        } else if (key == "androidboot.fstab_suffix") {
            strscpy(fstab_suffix.data(), value.data(), fstab_suffix.size());
        } else if (key == "qemu") {
            emulator = true;
        } else if (key == "androidboot.partition_map") {
            // androidboot.partition_map allows mapping a partition name to a raw block device.
            // For example, "androidboot.partition_map=vdb,metadata;vdc,userdata" maps
            // "vdb" to "metadata", and "vdc" to "userdata".
            // https://android.googlesource.com/platform/system/core/+/refs/heads/android13-release/init/devices.cpp#191
            for (const auto &[k, v]: parse_partition_map(value)) {
                partition_map.emplace_back(k, v);
            }
        }
    }
}

#define read_dt(name, key)                                          \
ssprintf(file_name, sizeof(file_name), "%s/" name, dt_dir.data());  \
if (access(file_name, R_OK) == 0) {                                 \
    string data = full_read(file_name);                             \
    if (!data.empty()) {                                            \
        data.pop_back();                                            \
        strscpy(key.data(), data.data(), key.size());               \
    }                                                               \
}

void BootConfig::init() noexcept {
    set(parse_cmdline(full_read("/proc/cmdline")));
    set(parse_bootconfig(full_read("/proc/bootconfig")));

    parse_prop_file("/.backup/.magisk", [&](auto key, auto value) -> bool {
        if (key == "RECOVERYMODE" && value == "true") {
            skip_initramfs = emulator || !check_key_combo();
            return false;
        }
        return true;
    });

    if (dt_dir[0] == '\0')
        strscpy(dt_dir.data(), DEFAULT_DT_DIR, dt_dir.size());

    char file_name[128];
    read_dt("fstab_suffix", fstab_suffix)
    read_dt("hardware", hardware)
    read_dt("hardware.platform", hardware_plat)

    LOGD("Device config:\n");
    print();
}

bool MagiskInit::check_two_stage() const noexcept {
    if (access("/first_stage_ramdisk", F_OK) == 0)
        return true;
    if (access("/second_stage_resources", F_OK) == 0)
        return true;
    if (access("/system/bin/init", F_OK) == 0)
        return true;
    // Use the apex folder to determine whether 2SI (Android 10+)
    if (access("/apex", F_OK) == 0)
        return true;
    // If we still have no indication, parse the original init and see what's up
    mmap_data init(backup_init());
    return init.contains("selinux_setup");
}
