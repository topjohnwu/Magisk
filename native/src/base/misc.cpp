#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include <syscall.h>
#include <random>
#include <string>

#include <base.hpp>

using namespace std;

int fork_dont_care() {
    if (int pid = xfork()) {
        waitpid(pid, nullptr, 0);
        return pid;
    } else if (xfork()) {
        exit(0);
    }
    return 0;
}

int fork_no_orphan() {
    int pid = xfork();
    if (pid)
        return pid;
    prctl(PR_SET_PDEATHSIG, SIGKILL);
    if (getppid() == 1)
        exit(1);
    return 0;
}

int gen_rand_str(char *buf, int len, bool varlen) {
    constexpr char ALPHANUM[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::mt19937 gen([]{
        if (access("/dev/urandom", F_OK) != 0)
            mknod("/dev/urandom", 0600 | S_IFCHR, makedev(1, 9));
        int fd = xopen("/dev/urandom", O_RDONLY | O_CLOEXEC);
        unsigned seed;
        xxread(fd, &seed, sizeof(seed));
        return seed;
    }());
    std::uniform_int_distribution<int> dist(0, sizeof(ALPHANUM) - 2);
    if (varlen) {
        std::uniform_int_distribution<int> len_dist(len / 2, len);
        len = len_dist(gen);
    }
    for (int i = 0; i < len - 1; ++i)
        buf[i] = ALPHANUM[dist(gen)];
    buf[len - 1] = '\0';
    return len - 1;
}

int exec_command(exec_t &exec) {
    auto pipefd = array<int, 2>{-1, -1};
    int outfd = -1;

    if (exec.fd == -1) {
        if (xpipe2(pipefd, O_CLOEXEC) == -1)
            return -1;
        outfd = pipefd[1];
    } else if (exec.fd >= 0) {
        outfd = exec.fd;
    }

    int pid = exec.fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    } else if (pid) {
        if (exec.fd == -1) {
            exec.fd = pipefd[0];
            close(pipefd[1]);
        }
        return pid;
    }

    // Unblock all signals
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_UNBLOCK, &set, nullptr);

    if (outfd >= 0) {
        xdup2(outfd, STDOUT_FILENO);
        if (exec.err)
            xdup2(outfd, STDERR_FILENO);
        close(outfd);
    }

    // Call the pre-exec callback
    if (exec.pre_exec)
        exec.pre_exec();

    if (exec.ns_pid > 0)
        switch_mnt_ns(exec.ns_pid);

    execve(exec.argv[0], (char **) exec.argv, environ);
    PLOGE("execve %s", exec.argv[0]);
    exit(-1);
}

int exec_command_sync(exec_t &exec) {
    int pid = exec_command(exec);
    if (pid < 0)
        return -1;
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

int new_daemon_thread(thread_entry entry, void *arg) {
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    errno = pthread_create(&thread, &attr, entry, arg);
    if (errno) {
        PLOGE("pthread_create");
    }
    return errno;
}

static char *argv0;
static size_t name_len;
void init_argv0(int argc, char **argv) {
    argv0 = argv[0];
    name_len = (argv[argc - 1] - argv[0]) + strlen(argv[argc - 1]) + 1;
}

void set_nice_name(const char *name) {
    memset(argv0, 0, name_len);
    strscpy(argv0, name, name_len);
    prctl(PR_SET_NAME, name);
}

/*
 * Bionic's atoi runs through strtol().
 * Use our own implementation for faster conversion.
 */
int parse_int(string_view s) {
    int val = 0;
    for (char c : s) {
        if (!c) break;
        if (c > '9' || c < '0')
            return -1;
        val = val * 10 + c - '0';
    }
    return val;
}

uint32_t binary_gcd(uint32_t u, uint32_t v) {
    if (u == 0) return v;
    if (v == 0) return u;
    auto shift = __builtin_ctz(u | v);
    u >>= __builtin_ctz(u);
    do {
        v >>= __builtin_ctz(v);
        if (u > v) {
            auto t = v;
            v = u;
            u = t;
        }
        v -= u;
    } while (v != 0);
    return u << shift;
}

int switch_mnt_ns(int pid) {
    char mnt[32];
    ssprintf(mnt, sizeof(mnt), "/proc/%d/ns/mnt", pid);
    if (access(mnt, R_OK) == -1) return 1; // Maybe process died..

    int fd, ret;
    fd = xopen(mnt, O_RDONLY);
    if (fd < 0) return 1;
    // Switch to its namespace
    ret = xsetns(fd, 0);
    close(fd);
    return ret;
}

string &replace_all(string &str, string_view from, string_view to) {
    size_t pos = 0;
    while((pos = str.find(from, pos)) != string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}

template <class T>
static auto split_impl(T s, T delims) {
    vector<std::decay_t<T>> result;
    size_t base = 0;
    size_t found;
    while (true) {
        found = s.find_first_of(delims, base);
        result.push_back(s.substr(base, found - base));
        if (found == string::npos)
            break;
        base = found + 1;
    }
    return result;
}

vector<string> split(const string &s, const string &delims) {
    return split_impl<const string&>(s, delims);
}

vector<string_view> split_ro(string_view s, string_view delims) {
    return split_impl<string_view>(s, delims);
}

#undef vsnprintf
int vssprintf(char *dest, size_t size, const char *fmt, va_list ap) {
    if (size > 0) {
        *dest = 0;
        return std::min(vsnprintf(dest, size, fmt, ap), (int) size - 1);
    }
    return -1;
}

int ssprintf(char *dest, size_t size, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int r = vssprintf(dest, size, fmt, va);
    va_end(va);
    return r;
}

#undef strlcpy
size_t strscpy(char *dest, const char *src, size_t size) {
    return std::min(strlcpy(dest, src, size), size - 1);
}

std::vector<MountInfo> ParseMountInfo(const char *pid) {
    char buf[PATH_MAX] = {};
    ssprintf(buf, sizeof(buf), "/proc/%s/mountinfo", pid);
    auto mount_info = xopen_file(buf, "r");
    char *line = nullptr;
    run_finally free_line([&line] { free(line); });
    size_t len = 0;
    ssize_t nread;

    std::vector<MountInfo> result;

    while ((nread = getline(&line, &len, mount_info.get())) != -1) {
        if (line[nread - 1] == '\n')
            line[nread - 1] = '\0';
        int root_start = 0, root_end = 0;
        int target_start = 0, target_end = 0;
        int vfs_option_start = 0, vfs_option_end = 0;
        int type_start = 0, type_end = 0;
        int source_start = 0, source_end = 0;
        int fs_option_start = 0, fs_option_end = 0;
        int optional_start = 0, optional_end = 0;
        unsigned int id, parent, maj, min;
        sscanf(line,
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
        std::string_view line_view(line, nread - 1);

        auto root = line_view.substr(root_start, root_end - root_start);
        auto target = line_view.substr(target_start, target_end - target_start);
        auto vfs_option =
                line_view.substr(vfs_option_start, vfs_option_end - vfs_option_start);
        ++optional_start;
        --optional_end;
        auto optional = line_view.substr(
                optional_start,
                optional_end - optional_start > 0 ? optional_end - optional_start : 0);

        auto type = line_view.substr(type_start, type_end - type_start);
        auto source = line_view.substr(source_start, source_end - source_start);
        auto fs_option =
                line_view.substr(fs_option_start, fs_option_end - fs_option_start);

        unsigned int shared = 0;
        unsigned int master = 0;
        unsigned int propagate_from = 0;
        if (auto pos = optional.find("shared:"); pos != std::string_view::npos) {
            shared = strtoul(optional.data() + pos + 7, nullptr, 10);
        }
        if (auto pos = optional.find("master:"); pos != std::string_view::npos) {
            master = strtoul(optional.data() + pos + 7, nullptr, 10);
        }
        if (auto pos = optional.find("propagate_from:");
                pos != std::string_view::npos) {
            propagate_from = strtoul(optional.data() + pos + 15, nullptr, 10);
        }

        result.emplace_back(MountInfo{
                .id = id,
                .parent = parent,
                .device = static_cast<dev_t>(makedev(maj, min)),
                .root = std::string(root),
                .target = std::string(target),
                .vfs_option = std::string(vfs_option),
                .optional =
                        {
                                .shared = shared,
                                .master = master,
                                .propagate_from = propagate_from,
                        },
                .type = std::string(type),
                .source = std::string(source),
                .fs_option = std::string(fs_option),
        });
    }
    return result;
}
