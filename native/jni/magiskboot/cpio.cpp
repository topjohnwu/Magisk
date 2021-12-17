#include <unistd.h>
#include <fcntl.h>
#include <algorithm>

#include <utils.hpp>

#include "cpio.hpp"

using namespace std;

struct cpio_newc_header {
    char magic[6];
    char ino[8];
    char mode[8];
    char uid[8];
    char gid[8];
    char nlink[8];
    char mtime[8];
    char filesize[8];
    char devmajor[8];
    char devminor[8];
    char rdevmajor[8];
    char rdevminor[8];
    char namesize[8];
    char check[8];
} __attribute__((packed));

static uint32_t x8u(const char *hex) {
    uint32_t val, inpos = 8, outpos;
    char pattern[6];

    while (*hex == '0') {
        hex++;
        if (!--inpos) return 0;
    }
    // Because scanf gratuitously treats %*X differently than printf does.
    sprintf(pattern, "%%%dx%%n", inpos);
    sscanf(hex, pattern, &val, &outpos);
    if (inpos != outpos)
        LOGE("bad cpio header\n");

    return val;
}

cpio_entry::cpio_entry(uint32_t mode) : mode(mode), uid(0), gid(0), filesize(0), data(nullptr) {}

cpio_entry::cpio_entry(const cpio_newc_header *h) :
mode(x8u(h->mode)), uid(x8u(h->uid)), gid(x8u(h->gid)), filesize(x8u(h->filesize)), data(nullptr)
{}

void cpio::dump(const char *file) {
    fprintf(stderr, "Dump cpio: [%s]\n", file);
    dump(xfopen(file, "we"));
}

void cpio::rm(entry_map::iterator &it) {
    fprintf(stderr, "Remove [%s]\n", it->first.data());
    entries.erase(it);
}

void cpio::rm(const char *name, bool r) {
    size_t len = strlen(name);
    for (auto it = entries.begin(); it != entries.end();) {
        if (it->first.compare(0, len, name) == 0 &&
            ((r && it->first[len] == '/') || it->first[len] == '\0')) {
            auto tmp = it;
            ++it;
            rm(tmp);
            if (!r) return;
        } else {
            ++it;
        }
    }
}

void cpio::extract_entry(const entry_map::value_type &e, const char *file) {
    fprintf(stderr, "Extract [%s] to [%s]\n", e.first.data(), file);
    unlink(file);
    rmdir(file);
    if (S_ISDIR(e.second->mode)) {
        ::mkdir(file, e.second->mode & 0777);
    } else if (S_ISREG(e.second->mode)) {
        int fd = creat(file, e.second->mode & 0777);
        xwrite(fd, e.second->data, e.second->filesize);
        fchown(fd, e.second->uid, e.second->gid);
        close(fd);
    } else if (S_ISLNK(e.second->mode)) {
        auto target = strndup((char *) e.second->data, e.second->filesize);
        symlink(target, file);
        free(target);
    }
}

void cpio::extract() {
    for (auto &e : entries)
        extract_entry(e, e.first.data());
}

bool cpio::extract(const char *name, const char *file) {
    auto it = entries.find(name);
    if (it != entries.end()) {
        extract_entry(*it, file);
        return true;
    }
    fprintf(stderr, "Cannot find the file entry [%s]\n", name);
    return false;
}

bool cpio::exists(const char *name) {
    return entries.count(name) != 0;
}

#define do_out(buf, len) pos += fwrite(buf, 1, len, out);
#define out_align() do_out(zeros, align_padding(pos, 4))
void cpio::dump(FILE *out) {
    size_t pos = 0;
    unsigned inode = 300000;
    char header[111];
    char zeros[4] = {0};
    for (auto &e : entries) {
        sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
                inode++,    // e->ino
                e.second->mode,
                e.second->uid,
                e.second->gid,
                1,          // e->nlink
                0,          // e->mtime
                e.second->filesize,
                0,          // e->devmajor
                0,          // e->devminor
                0,          // e->rdevmajor
                0,          // e->rdevminor
                (uint32_t) e.first.size() + 1,
                0           // e->check
        );
        do_out(header, 110);
        do_out(e.first.data(), e.first.size() + 1);
        out_align();
        if (e.second->filesize) {
            do_out(e.second->data, e.second->filesize);
            out_align();
        }
    }
    // Write trailer
    sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
            inode++, 0755, 0, 0, 1, 0, 0, 0, 0, 0, 0, 11, 0);
    do_out(header, 110);
    do_out("TRAILER!!!\0", 11);
    out_align();
    fclose(out);
}

void cpio::load_cpio(const char *file) {
    fprintf(stderr, "Loading cpio: [%s]\n", file);
    auto m = mmap_data(file);
    load_cpio(reinterpret_cast<char *>(m.buf), m.sz);
}

void cpio::insert(string_view name, cpio_entry *e) {
    auto it = entries.find(name);
    if (it != entries.end()) {
        it->second.reset(e);
    } else {
        entries.emplace(name, e);
    }
}

void cpio::add(mode_t mode, const char *name, const char *file) {
    auto m = mmap_data(file);
    auto e = new cpio_entry(S_IFREG | mode);
    e->filesize = m.sz;
    e->data = xmalloc(m.sz);
    memcpy(e->data, m.buf, m.sz);
    insert(name, e);
    fprintf(stderr, "Add entry [%s] (%04o)\n", name, mode);
}

void cpio::mkdir(mode_t mode, const char *name) {
    insert(name, new cpio_entry(S_IFDIR | mode));
    fprintf(stderr, "Create directory [%s] (%04o)\n", name, mode);
}

void cpio::ln(const char *target, const char *name) {
    auto e = new cpio_entry(S_IFLNK);
    e->filesize = strlen(target);
    e->data = strdup(target);
    insert(name, e);
    fprintf(stderr, "Create symlink [%s] -> [%s]\n", name, target);
}

void cpio::mv(entry_map::iterator &it, const char *name) {
    fprintf(stderr, "Move [%s] -> [%s]\n", it->first.data(), name);
    auto e = it->second.release();
    entries.erase(it);
    insert(name, e);
}

bool cpio::mv(const char *from, const char *to) {
    auto it = entries.find(from);
    if (it != entries.end()) {
        mv(it, to);
        return true;
    }
    fprintf(stderr, "Cannot find entry %s\n", from);
    return false;
}

#define pos_align(p) p = align_to(p, 4)

void cpio::load_cpio(const char *buf, size_t sz) {
    size_t pos = 0;
    while (pos < sz) {
        auto header = reinterpret_cast<const cpio_newc_header *>(buf + pos);
        pos += sizeof(cpio_newc_header);
        string_view name(buf + pos);
        pos += x8u(header->namesize);
        pos_align(pos);
        if (name == "." || name == "..")
            continue;
        if (name == "TRAILER!!!")
            break;
        auto entry = new cpio_entry(header);
        entry->data = xmalloc(entry->filesize);
        memcpy(entry->data, buf + pos, entry->filesize);
        pos += entry->filesize;
        insert(name, entry);
        pos_align(pos);
    }
}
