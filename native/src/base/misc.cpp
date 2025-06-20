#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <syscall.h>
#include <random>
#include <string>

#include <base.hpp>

using namespace std;

bool byte_view::contains(byte_view pattern) const {
    return _buf != nullptr && memmem(_buf, _sz, pattern._buf, pattern._sz) != nullptr;
}

bool byte_view::equals(byte_view o) const {
    return _sz == o._sz && memcmp(_buf, o._buf, _sz) == 0;
}

heap_data byte_view::clone() const {
    heap_data copy(_sz);
    memcpy(copy._buf, _buf, _sz);
    return copy;
}

void byte_data::swap(byte_data &o) {
    std::swap(_buf, o._buf);
    std::swap(_sz, o._sz);
}

rust::Vec<size_t> byte_data::patch(byte_view from, byte_view to) {
    rust::Vec<size_t> v;
    if (_buf == nullptr)
        return v;
    auto p = _buf;
    auto eof = _buf + _sz;
    while (p < eof) {
        p = static_cast<uint8_t *>(memmem(p, eof - p, from.buf(), from.sz()));
        if (p == nullptr)
            return v;
        memset(p, 0, from.sz());
        memcpy(p, to.buf(), to.sz());
        v.push_back(p - _buf);
        p += from.sz();
    }
    return v;
}

rust::Vec<size_t> mut_u8_patch(
        rust::Slice<uint8_t> buf,
        rust::Slice<const uint8_t> from,
        rust::Slice<const uint8_t> to) {
    byte_data data(buf);
    return data.patch(from, to);
}

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

template<typename T, int base>
static T parse_num(string_view s) {
    T val = 0;
    for (char c : s) {
        if (isdigit(c)) {
            c -= '0';
        } else if (base > 10 && isalpha(c)) {
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        } else {
            return -1;
        }
        if (c >= base) {
            return -1;
        }
        val *= base;
        val += c;
    }
    return val;
}

/*
 * Bionic's atoi runs through strtol().
 * Use our own implementation for faster conversion.
 */
int parse_int(string_view s) {
    return parse_num<int, 10>(s);
}

uint32_t parse_uint32_hex(string_view s) {
    return parse_num<uint32_t, 16>(s);
}

int switch_mnt_ns(int pid) {
    int ret = -1;
    int fd = syscall(__NR_pidfd_open, pid, 0);
    if (fd > 0) {
        ret = setns(fd, CLONE_NEWNS);
        close(fd);
    }
    if (ret < 0) {
        char mnt[32];
        ssprintf(mnt, sizeof(mnt), "/proc/%d/ns/mnt", pid);
        fd = open(mnt, O_RDONLY);
        if (fd < 0) return 1; // Maybe process died..

        // Switch to its namespace
        ret = xsetns(fd, 0);
        close(fd);
    }
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

template <typename T>
static auto split_impl(string_view s, string_view delims) {
    vector<T> result;
    size_t base = 0;
    size_t found;
    while (true) {
        found = s.find_first_of(delims, base);
        result.emplace_back(s.substr(base, found - base));
        if (found == string::npos)
            break;
        base = found + 1;
    }
    return result;
}

vector<string> split(string_view s, string_view delims) {
    return split_impl<string>(s, delims);
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

extern "C" void cxx$utf8str$new(rust::Utf8CStr *self, const void *s, size_t len);
extern "C" const char *cxx$utf8str$ptr(const rust::Utf8CStr *self);
extern "C" size_t cxx$utf8str$len(const rust::Utf8CStr *self);

rust::Utf8CStr::Utf8CStr(const char *s, size_t len) {
    cxx$utf8str$new(this, s, len);
}

const char *rust::Utf8CStr::data() const {
    return cxx$utf8str$ptr(this);
}

size_t rust::Utf8CStr::length() const {
    return cxx$utf8str$len(this);
}
