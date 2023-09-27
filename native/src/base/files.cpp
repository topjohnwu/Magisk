#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/sysmacros.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>

#include <base.hpp>
#include <selinux.hpp>

using namespace std;

int fd_pathat(int dirfd, const char *name, char *path, size_t size) {
    if (fd_path(dirfd, byte_data(path, size)) < 0)
        return -1;
    auto len = strlen(path);
    path[len] = '/';
    strscpy(path + len + 1, name, size - len - 1);
    return 0;
}

void full_read(int fd, string &str) {
    char buf[4096];
    for (ssize_t len; (len = xread(fd, buf, sizeof(buf))) > 0;)
        str.insert(str.end(), buf, buf + len);
}

void full_read(const char *filename, string &str) {
    if (int fd = xopen(filename, O_RDONLY | O_CLOEXEC); fd >= 0) {
        full_read(fd, str);
        close(fd);
    }
}

string full_read(int fd) {
    string str;
    full_read(fd, str);
    return str;
}

string full_read(const char *filename) {
    string str;
    full_read(filename, str);
    return str;
}

void write_zero(int fd, size_t size) {
    char buf[4096] = {0};
    size_t len;
    while (size > 0) {
        len = sizeof(buf) > size ? size : sizeof(buf);
        write(fd, buf, len);
        size -= len;
    }
}

void file_readline(bool trim, FILE *fp, const function<bool(string_view)> &fn) {
    size_t len = 1024;
    char *buf = (char *) malloc(len);
    char *start;
    ssize_t read;
    while ((read = getline(&buf, &len, fp)) >= 0) {
        start = buf;
        if (trim) {
            while (read && "\n\r "sv.find(buf[read - 1]) != string::npos)
                --read;
            buf[read] = '\0';
            while (*start == ' ')
                ++start;
        }
        if (!fn(start))
            break;
    }
    free(buf);
}

void file_readline(bool trim, const char *file, const function<bool(string_view)> &fn) {
    if (auto fp = open_file(file, "re"))
        file_readline(trim, fp.get(), fn);
}
void file_readline(const char *file, const function<bool(string_view)> &fn) {
    file_readline(false, file, fn);
}

void parse_prop_file(FILE *fp, const function<bool(string_view, string_view)> &fn) {
    file_readline(true, fp, [&](string_view line_view) -> bool {
        char *line = (char *) line_view.data();
        if (line[0] == '#')
            return true;
        char *eql = strchr(line, '=');
        if (eql == nullptr || eql == line)
            return true;
        *eql = '\0';
        return fn(line, eql + 1);
    });
}

void parse_prop_file(const char *file, const function<bool(string_view, string_view)> &fn) {
    if (auto fp = open_file(file, "re"))
        parse_prop_file(fp.get(), fn);
}

std::vector<mount_info> parse_mount_info(const char *pid) {
    char buf[PATH_MAX] = {};
    ssprintf(buf, sizeof(buf), "/proc/%s/mountinfo", pid);
    std::vector<mount_info> result;

    file_readline(buf, [&result](string_view line) -> bool {
        int root_start = 0, root_end = 0;
        int target_start = 0, target_end = 0;
        int vfs_option_start = 0, vfs_option_end = 0;
        int type_start = 0, type_end = 0;
        int source_start = 0, source_end = 0;
        int fs_option_start = 0, fs_option_end = 0;
        int optional_start = 0, optional_end = 0;
        unsigned int id, parent, maj, min;
        sscanf(line.data(),
               "%u "           // (1) id
               "%u "           // (2) parent
               "%u:%u "        // (3) maj:min
               "%n%*s%n "      // (4) mountroot
               "%n%*s%n "      // (5) target
               "%n%*s%n"       // (6) vfs options (fs-independent)
               "%n%*[^-]%n - " // (7) optional fields
               "%n%*s%n "      // (8) FS type
               "%n%*s%n "      // (9) source
               "%n%*s%n",      // (10) fs options (fs specific)
               &id, &parent, &maj, &min, &root_start, &root_end, &target_start,
               &target_end, &vfs_option_start, &vfs_option_end,
               &optional_start, &optional_end, &type_start, &type_end,
               &source_start, &source_end, &fs_option_start, &fs_option_end);

        auto root = line.substr(root_start, root_end - root_start);
        auto target = line.substr(target_start, target_end - target_start);
        auto vfs_option =
                line.substr(vfs_option_start, vfs_option_end - vfs_option_start);
        ++optional_start;
        --optional_end;
        auto optional = line.substr(
                optional_start,
                optional_end - optional_start > 0 ? optional_end - optional_start : 0);

        auto type = line.substr(type_start, type_end - type_start);
        auto source = line.substr(source_start, source_end - source_start);
        auto fs_option =
                line.substr(fs_option_start, fs_option_end - fs_option_start);

        unsigned int shared = 0;
        unsigned int master = 0;
        unsigned int propagate_from = 0;
        if (auto pos = optional.find("shared:"); pos != std::string_view::npos) {
            shared = parse_int(optional.substr(pos + 7));
        }
        if (auto pos = optional.find("master:"); pos != std::string_view::npos) {
            master = parse_int(optional.substr(pos + 7));
        }
        if (auto pos = optional.find("propagate_from:");
                pos != std::string_view::npos) {
            propagate_from = parse_int(optional.substr(pos + 15));
        }

        result.emplace_back(mount_info {
                .id = id,
                .parent = parent,
                .device = static_cast<dev_t>(makedev(maj, min)),
                .root {root},
                .target {target},
                .vfs_option {vfs_option},
                .optional {
                        .shared = shared,
                        .master = master,
                        .propagate_from = propagate_from,
                },
                .type {type},
                .source {source},
                .fs_option {fs_option},
        });
        return true;
    });
    return result;
}

sDIR make_dir(DIR *dp) {
    return sDIR(dp, [](DIR *dp){ return dp ? closedir(dp) : 1; });
}

sFILE make_file(FILE *fp) {
    return sFILE(fp, [](FILE *fp){ return fp ? fclose(fp) : 1; });
}

mmap_data::mmap_data(const char *name, bool rw) {
    auto slice = rust::map_file(byte_view(name), rw);
    if (!slice.empty()) {
        _buf = slice.data();
        _sz = slice.size();
    }
}

mmap_data::mmap_data(int fd, size_t sz, bool rw) {
    auto slice = rust::map_fd(fd, sz, rw);
    if (!slice.empty()) {
        _buf = slice.data();
        _sz = slice.size();
    }
}

mmap_data::~mmap_data() {
    if (_buf)
        munmap(_buf, _sz);
}

string resolve_preinit_dir(const char *base_dir) {
    string dir = base_dir;
    if (access((dir + "/unencrypted").data(), F_OK) == 0) {
        dir += "/unencrypted/magisk";
    } else if (access((dir + "/adb").data(), F_OK) == 0) {
        dir += "/adb/modules";
    } else {
        dir += "/magisk";
    }
    return dir;
}
