#include <sys/uio.h>
#include <android/log.h>

#include <magisk.hpp>
#include <base.hpp>
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

atomic<int> logd_fd = -1;

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
#define MAX_MSG_LEN  (int) (PIPE_BUF - sizeof(log_meta))

static void *logfile_writer(void *arg) {
    int pipefd = (long) arg;

    run_finally close_pipes([=] {
        // Close up all logging pipes when thread dies
        close(pipefd);
        close(logd_fd.exchange(-1));
    });

    struct {
        void *data;
        size_t len;
    } tmp{};
    stream_ptr strm = make_unique<byte_stream>(tmp.data, tmp.len);
    bool switched = false;

    log_meta meta{};
    char buf[MAX_MSG_LEN];
    char aux[64];

    iovec iov[2];
    iov[0].iov_base = aux;
    iov[1].iov_base = buf;

    for (;;) {
        // Read meta data
        if (read(pipefd, &meta, sizeof(meta)) != sizeof(meta))
            return nullptr;

        if (meta.prio < 0) {
            if (!switched) {
                run_finally free_tmp([&] {
                    free(tmp.data);
                    tmp.data = nullptr;
                    tmp.len = 0;
                });

                rename(LOGFILE, LOGFILE ".bak");
                int fd = open(LOGFILE, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0644);
                if (fd < 0)
                    return nullptr;
                if (tmp.data)
                    write(fd, tmp.data, tmp.len);

                strm = make_unique<fd_stream>(fd);
                switched = true;
            }
            continue;
        }

        // Read message
        if (read(pipefd, buf, meta.len) != meta.len)
            return nullptr;

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
        long ms = tv.tv_usec / 1000;
        size_t off = strftime(aux, sizeof(aux), "%m-%d %T", &tm);
        off += ssprintf(aux + off, sizeof(aux) - off,
                ".%03ld %5d %5d %c : ", ms, meta.pid, meta.tid, type);

        iov[0].iov_len = off;
        iov[1].iov_len = meta.len;

        strm->writev(iov, 2);
    }
}

void magisk_log_write(int prio, const char *msg, int len) {
    if (logd_fd < 0)
        return;

    // Truncate
    len = std::min(MAX_MSG_LEN, len);

    log_meta meta = {
        .prio = prio,
        .len = len,
        .pid = getpid(),
        .tid = gettid()
    };

    iovec iov[2];
    iov[0].iov_base = &meta;
    iov[0].iov_len = sizeof(meta);
    iov[1].iov_base = (void *) msg;
    iov[1].iov_len = len;

    if (writev(logd_fd, iov, 2) < 0) {
        // Stop trying to write to file
        close(logd_fd.exchange(-1));
    }
}

void start_log_daemon() {
    int fds[2];
    if (pipe2(fds, O_CLOEXEC) == 0) {
        logd_fd = fds[1];
        long fd = fds[0];
        new_daemon_thread(logfile_writer, (void *) fd);
    }
}
