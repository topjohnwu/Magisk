#include <sys/mman.h>
#include <sys/sendfile.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <unistd.h>

#include <base.hpp>

using namespace std;

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

sDIR make_dir(DIR *dp) {
    return sDIR(dp, [](DIR *dp){ return dp ? closedir(dp) : 1; });
}

sFILE make_file(FILE *fp) {
    return sFILE(fp, [](FILE *fp){ return fp ? fclose(fp) : 1; });
}

mmap_data::mmap_data(const char *name, bool rw) {
    auto slice = rust::map_file(name, rw);
    if (!slice.empty()) {
        _buf = slice.data();
        _sz = slice.size();
    }
}

mmap_data::mmap_data(int dirfd, const char *name, bool rw) {
    auto slice = rust::map_file_at(dirfd, name, rw);
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
        dir += "/adb";
    } else if (access((dir + "/watchdog").data(), F_OK) == 0) {
        dir += "/watchdog/magisk";
    } else {
        dir += "/magisk";
    }
    return dir;
}
