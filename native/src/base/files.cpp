#include <sys/sendfile.h>
#include <sys/sysmacros.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>

#include <base.hpp>
#include <misc.hpp>
#include <selinux.hpp>

using namespace std;

int fd_pathat(int dirfd, const char *name, char *path, size_t size) {
    if (fd_path(dirfd, byte_slice(path, size)) < 0)
        return -1;
    auto len = strlen(path);
    path[len] = '/';
    strscpy(path + len + 1, name, size - len - 1);
    return 0;
}

template <typename Func>
static void post_order_walk(int dirfd, const Func &fn) {
    auto dir = xopen_dir(dirfd);
    if (!dir) return;

    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        if (entry->d_type == DT_DIR)
            post_order_walk(xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC), fn);
        fn(dirfd, entry);
    }
}

enum walk_result {
    CONTINUE, SKIP, ABORT
};

template <typename Func>
static walk_result pre_order_walk(int dirfd, const Func &fn) {
    auto dir = xopen_dir(dirfd);
    if (!dir) {
        close(dirfd);
        return SKIP;
    }

    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        switch (fn(dirfd, entry)) {
        case CONTINUE:
            break;
        case SKIP:
            continue;
        case ABORT:
            return ABORT;
        }
        if (entry->d_type == DT_DIR) {
            int fd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
            if (pre_order_walk(fd, fn) == ABORT)
                return ABORT;
        }
    }
    return CONTINUE;
}

static void remove_at(int dirfd, struct dirent *entry) {
    unlinkat(dirfd, entry->d_name, entry->d_type == DT_DIR ? AT_REMOVEDIR : 0);
}

void rm_rf(const char *path) {
    struct stat st;
    if (lstat(path, &st) < 0)
        return;
    if (S_ISDIR(st.st_mode))
        frm_rf(xopen(path, O_RDONLY | O_CLOEXEC));
    remove(path);
}

void frm_rf(int dirfd) {
    post_order_walk(dirfd, remove_at);
}

void mv_path(const char *src, const char *dest) {
    file_attr attr;
    getattr(src, &attr);
    if (S_ISDIR(attr.st.st_mode)) {
        if (access(dest, F_OK) != 0) {
            xmkdirs(dest, 0);
            setattr(dest, &attr);
        }
        mv_dir(xopen(src, O_RDONLY | O_CLOEXEC), xopen(dest, O_RDONLY | O_CLOEXEC));
    } else{
        xrename(src, dest);
    }
    rmdir(src);
}

void mv_dir(int src, int dest) {
    auto dir = xopen_dir(src);
    run_finally f([=]{ close(dest); });
    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        switch (entry->d_type) {
        case DT_DIR:
            if (xfaccessat(dest, entry->d_name, F_OK, 0) == 0) {
                // Destination folder exists, needs recursive move
                int newsrc = xopenat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
                int newdest = xopenat(dest, entry->d_name, O_RDONLY | O_CLOEXEC);
                mv_dir(newsrc, newdest);
                unlinkat(src, entry->d_name, AT_REMOVEDIR);
                break;
            }
            // Else fall through
        case DT_LNK:
        case DT_REG:
            renameat(src, entry->d_name, dest, entry->d_name);
            break;
        }
    }
}

void cp_afc(const char *src, const char *dest) {
    file_attr a;
    getattr(src, &a);

    if (S_ISDIR(a.st.st_mode)) {
        xmkdirs(dest, 0);
        clone_dir(xopen(src, O_RDONLY | O_CLOEXEC), xopen(dest, O_RDONLY | O_CLOEXEC));
    } else{
        unlink(dest);
        if (S_ISREG(a.st.st_mode)) {
            int sfd = xopen(src, O_RDONLY | O_CLOEXEC);
            int dfd = xopen(dest, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0);
            xsendfile(dfd, sfd, nullptr, a.st.st_size);
            close(sfd);
            close(dfd);
        } else if (S_ISLNK(a.st.st_mode)) {
            char buf[4096];
            xreadlink(src, buf, sizeof(buf));
            xsymlink(buf, dest);
        }
    }
    setattr(dest, &a);
}

void clone_dir(int src, int dest) {
    auto dir = xopen_dir(src);
    run_finally f([&]{ close(dest); });
    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        file_attr a;
        getattrat(src, entry->d_name, &a);
        switch (entry->d_type) {
            case DT_DIR: {
                xmkdirat(dest, entry->d_name, 0);
                setattrat(dest, entry->d_name, &a);
                int sfd = xopenat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
                int dst = xopenat(dest, entry->d_name, O_RDONLY | O_CLOEXEC);
                clone_dir(sfd, dst);
                break;
            }
            case DT_REG: {
                int sfd = xopenat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
                int dfd = xopenat(dest, entry->d_name, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0);
                xsendfile(dfd, sfd, nullptr, a.st.st_size);
                fsetattr(dfd, &a);
                close(dfd);
                close(sfd);
                break;
            }
            case DT_LNK: {
                char buf[4096];
                xreadlinkat(src, entry->d_name, buf, sizeof(buf));
                xsymlinkat(buf, dest, entry->d_name);
                setattrat(dest, entry->d_name, &a);
                break;
            }
        }
    }
}

void link_path(const char *src, const char *dest) {
    link_dir(xopen(src, O_RDONLY | O_CLOEXEC), xopen(dest, O_RDONLY | O_CLOEXEC));
}

void link_dir(int src, int dest) {
    auto dir = xopen_dir(src);
    run_finally f([&]{ close(dest); });
    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        if (entry->d_type == DT_DIR) {
            file_attr a;
            getattrat(src, entry->d_name, &a);
            xmkdirat(dest, entry->d_name, 0);
            setattrat(dest, entry->d_name, &a);
            int sfd = xopenat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
            int dfd = xopenat(dest, entry->d_name, O_RDONLY | O_CLOEXEC);
            link_dir(sfd, dfd);
        } else {
            xlinkat(src, entry->d_name, dest, entry->d_name, 0);
        }
    }
}

int getattr(const char *path, file_attr *a) {
    if (xlstat(path, &a->st) == -1)
        return -1;
    char *con;
    if (lgetfilecon(path, &con) == -1)
        return -1;
    strcpy(a->con, con);
    freecon(con);
    return 0;
}

int getattrat(int dirfd, const char *name, file_attr *a) {
    char path[4096];
    fd_pathat(dirfd, name, path, sizeof(path));
    return getattr(path, a);
}

int fgetattr(int fd, file_attr *a) {
    if (xfstat(fd, &a->st) < 0)
        return -1;
    char *con;
    if (fgetfilecon(fd, &con) < 0)
        return -1;
    strcpy(a->con, con);
    freecon(con);
    return 0;
}

int setattr(const char *path, file_attr *a) {
    if (chmod(path, a->st.st_mode & 0777) < 0)
        return -1;
    if (chown(path, a->st.st_uid, a->st.st_gid) < 0)
        return -1;
    if (a->con[0] && lsetfilecon(path, a->con) < 0)
        return -1;
    return 0;
}

int setattrat(int dirfd, const char *name, file_attr *a) {
    char path[4096];
    fd_pathat(dirfd, name, path, sizeof(path));
    return setattr(path, a);
}

int fsetattr(int fd, file_attr *a) {
    if (fchmod(fd, a->st.st_mode & 0777) < 0)
        return -1;
    if (fchown(fd, a->st.st_uid, a->st.st_gid) < 0)
        return -1;
    if (a->con[0] && fsetfilecon(fd, a->con) < 0)
        return -1;
    return 0;
}

void clone_attr(const char *src, const char *dest) {
    file_attr a;
    getattr(src, &a);
    setattr(dest, &a);
}

void fclone_attr(int src, int dest) {
    file_attr a;
    fgetattr(src, &a);
    fsetattr(dest, &a);
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

int byte_data::patch(bool log, str_pairs list) {
    if (buf == nullptr)
        return 0;
    int count = 0;
    for (uint8_t *p = buf, *eof = buf + sz; p < eof; ++p) {
        for (auto [from, to] : list) {
            if (memcmp(p, from.data(), from.length() + 1) == 0) {
                if (log) LOGD("Replace [%s] -> [%s]\n", from.data(), to.data());
                memset(p, 0, from.length());
                memcpy(p, to.data(), to.length());
                ++count;
                p += from.length();
            }
        }
    }
    return count;
}

bool byte_data::contains(string_view pattern, bool log) const {
    if (buf == nullptr)
        return false;
    for (uint8_t *p = buf, *eof = buf + sz; p < eof; ++p) {
        if (memcmp(p, pattern.data(), pattern.length() + 1) == 0) {
            if (log) LOGD("Found pattern [%s]\n", pattern.data());
            return true;
        }
    }
    return false;
}

void byte_data::swap(byte_data &o) {
    std::swap(buf, o.buf);
    std::swap(sz, o.sz);
}

mmap_data::mmap_data(const char *name, bool rw) {
    int fd = xopen(name, (rw ? O_RDWR : O_RDONLY) | O_CLOEXEC);
    if (fd < 0)
        return;
    struct stat st;
    if (fstat(fd, &st))
        return;
    if (S_ISBLK(st.st_mode)) {
        uint64_t size;
        ioctl(fd, BLKGETSIZE64, &size);
        sz = size;
    } else {
        sz = st.st_size;
    }
    void *b = sz > 0
            ? xmmap(nullptr, sz, PROT_READ | PROT_WRITE, rw ? MAP_SHARED : MAP_PRIVATE, fd, 0)
            : nullptr;
    close(fd);
    buf = static_cast<uint8_t *>(b);
}

string find_apk_path(const char *pkg) {
    char buf[PATH_MAX];
    size_t len = strlen(pkg);
    pre_order_walk(xopen("/data/app", O_RDONLY), [&](int dfd, dirent *entry) -> walk_result {
        if (entry->d_type != DT_DIR)
            return SKIP;
        if (strncmp(entry->d_name, pkg, len) == 0 && entry->d_name[len] == '-') {
            fd_pathat(dfd, entry->d_name, buf, sizeof(buf));
            return ABORT;
        } else if (strncmp(entry->d_name, "~~", 2) == 0) {
            return CONTINUE;
        } else return SKIP;
    });
    string path(buf);
    return path.append("/base.apk");
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
