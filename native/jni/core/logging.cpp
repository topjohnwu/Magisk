#include <sys/uio.h>
#include <android/log.h>

#include <magisk.hpp>
#include <utils.hpp>
#include <daemon.hpp>
#include <stream.hpp>

#include "core.hpp"

using namespace std;

struct log_meta {
    int prio;
    int len;
    int pid;
    int tid;
};

static atomic<int> logd_fd = -1;

void setup_logfile(bool reset) {
    if (logd_fd < 0)
        return;

    iovec iov{};
    log_meta meta = {
        .prio = -1,
        .len = reset
    };

    iov.iov_base = &meta;
    iov.iov_len = sizeof(meta);
    writev(logd_fd, &iov, 1);
}

// Maximum message length for pipes to transfer atomically
#define MAX_MSG_LEN  (PIPE_BUF - sizeof(log_meta))
#define TIME_FMT_LEN 35

static void logfile_writer(int pipefd) {
    run_finally close_socket([=] {
        // Close up all logging pipes when thread dies
        close(pipefd);
        close(logd_fd.exchange(-1));
    });

    struct {
        void *data;
        size_t len;
    } tmp_buf{};
    stream *strm = new byte_stream(tmp_buf.data, tmp_buf.len);

    log_meta meta{};
    char buf[MAX_MSG_LEN + TIME_FMT_LEN];

    for (;;) {
        // Read meta data
        if (xread(pipefd, &meta, sizeof(meta)) != sizeof(meta))
            return;

        if (meta.prio < 0 && tmp_buf.len >= 0) {
            run_finally free_buf([&] {
                free(tmp_buf.data);
                tmp_buf.data = nullptr;
                tmp_buf.len = -1;
            });

            if (meta.len)
                rename(LOGFILE, LOGFILE ".bak");

            int fd = open(LOGFILE, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0644);
            if (fd < 0)
                return;
            if (tmp_buf.data)
                write(fd, tmp_buf.data, tmp_buf.len);

            delete strm;
            strm = new fd_stream(fd);
            continue;
        }

        timeval tv;
        tm tm;
        gettimeofday(&tv, nullptr);
        localtime_r(&tv.tv_sec, &tm);

        // Format detailed info
        char type;
        switch (meta.prio) {
            case ANDROID_LOG_DEBUG:
                type = 'D';
                break;
            case ANDROID_LOG_INFO:
                type = 'I';
                break;
            case ANDROID_LOG_WARN:
                type = 'W';
                break;
            default:
                type = 'E';
                break;
        }
        size_t off = strftime(buf, sizeof(buf), "%m-%d %T", &tm);
        int ms = tv.tv_usec / 1000;
        off += snprintf(buf + off, sizeof(buf) - off,
                ".%03d %5d %5d %c : ", ms, meta.pid, meta.tid, type);

        // Read message
        if (xread(pipefd, buf + off, meta.len) != meta.len)
            return;

        strm->write(buf, off + meta.len);
    }
}

static int magisk_log(int prio, const char *fmt, va_list ap) {
    char buf[MAX_MSG_LEN + 1];
    int len = vsnprintf(buf, sizeof(buf), fmt, ap);

    if (logd_fd >= 0) {
        log_meta meta = {
            .prio = prio,
            .len = len,
            .pid = getpid(),
            .tid = gettid()
        };

        iovec iov[2];
        iov[0].iov_base = &meta;
        iov[0].iov_len = sizeof(meta);
        iov[1].iov_base = buf;
        iov[1].iov_len = len;

        if (writev(logd_fd, iov, 2) < 0) {
            // Stop trying to write to file
            close(logd_fd.exchange(-1));
        }
    }
    __android_log_write(prio, "Magisk", buf);

    return len;
}

#define mlog(prio) [](auto fmt, auto ap){ return magisk_log(ANDROID_LOG_##prio, fmt, ap); }
void magisk_logging() {
    log_cb.d = mlog(DEBUG);
    log_cb.i = mlog(INFO);
    log_cb.w = mlog(WARN);
    log_cb.e = mlog(ERROR);
    log_cb.ex = nop_ex;
}

void start_log_daemon() {
    int fds[2];
    if (pipe2(fds, O_CLOEXEC) == 0) {
        logd_fd = fds[1];
        new_daemon_thread([fd = fds[0]] { logfile_writer(fd); });
    }
}
