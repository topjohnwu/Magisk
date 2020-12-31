#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>

#include <utils.hpp>
#include <cpio.hpp>

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

cpio_entry_base::cpio_entry_base(const cpio_newc_header *h)
: mode(x8u(h->mode)), uid(x8u(h->uid)), gid(x8u(h->gid)), filesize(x8u(h->filesize)) {};

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

static void extract_entry(const entry_map::value_type &e, const char *file) {
    fprintf(stderr, "Extract [%s] to [%s]\n", e.first.data(), file);
    unlink(file);
    rmdir(file);
    if (S_ISDIR(e.second->mode)) {
        mkdir(file, e.second->mode & 0777);
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
#define out_align() do_out(zeros, align_off(pos, 4))
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

cpio_rw::cpio_rw(const char *file) {
    load_cpio(file);
}

void cpio_rw::load_cpio(const char *file) {
    char *buf;
    size_t sz;
    mmap_ro(file, buf, sz);
    fprintf(stderr, "Loading cpio: [%s]\n", file);
    load_cpio(buf, sz);
    munmap(buf, sz);
}

void cpio_rw::insert(cpio_entry *e) {
    auto ex = entries.extract(e->filename);
    if (!ex) {
        entries[e->filename].reset(e);
    } else {
        ex.key() = e->filename;
        ex.mapped().reset(e);
        entries.insert(std::move(ex));
    }
}

void cpio_rw::add(mode_t mode, const char *name, const char *file) {
    void *buf;
    size_t sz;
    mmap_ro(file, buf, sz);
    auto e = new cpio_entry(name, S_IFREG | mode);
    e->filesize = sz;
    e->data = xmalloc(sz);
    memcpy(e->data, buf, sz);
    munmap(buf, sz);
    insert(e);
    fprintf(stderr, "Add entry [%s] (%04o)\n", name, mode);
}

void cpio_rw::mkdir(mode_t mode, const char *name) {
    insert(new cpio_entry(name, S_IFDIR | mode));
    fprintf(stderr, "Create directory [%s] (%04o)\n", name, mode);
}

void cpio_rw::ln(const char *target, const char *name) {
    auto e = new cpio_entry(name, S_IFLNK);
    e->filesize = strlen(target);
    e->data = strdup(target);
    insert(e);
    fprintf(stderr, "Create symlink [%s] -> [%s]\n", name, target);
}

void cpio_rw::mv(entry_map::iterator &it, const char *to) {
    fprintf(stderr, "Move [%s] -> [%s]\n", it->first.data(), to);
    auto ex = entries.extract(it);
    auto &name = static_cast<cpio_entry*>(ex.mapped().get())->filename;
    name = to;
    ex.key() = name;
    entries.erase(name);
    entries.insert(std::move(ex));
}

bool cpio_rw::mv(const char *from, const char *to) {
    auto it = entries.find(from);
    if (it != entries.end()) {
        mv(it, to);
        return true;
    }
    fprintf(stderr, "Cannot find entry %s\n", from);
    return false;
}

#define pos_align(p) p = do_align(p, 4)

void cpio_rw::load_cpio(const char *buf, size_t sz) {
    size_t pos = 0;
    const cpio_newc_header *header;
    unique_ptr<cpio_entry> entry;
    while (pos < sz) {
        header = reinterpret_cast<const cpio_newc_header *>(buf + pos);
        entry = make_unique<cpio_entry>(header);
        pos += sizeof(*header);
        string_view name_view(buf + pos);
        pos += x8u(header->namesize);
        pos_align(pos);
        if (name_view == "." || name_view == "..")
            continue;
        if (name_view == "TRAILER!!!")
            break;
        entry->filename = name_view;
        entry->data = xmalloc(entry->filesize);
        memcpy(entry->data, buf + pos, entry->filesize);
        pos += entry->filesize;
        entries[entry->filename] = std::move(entry);
        pos_align(pos);
    }
}

cpio_mmap::cpio_mmap(const char *file) {
    mmap_ro(file, buf, sz);
    fprintf(stderr, "Loading cpio: [%s]\n", file);
    size_t pos = 0;
    cpio_newc_header *header;
    unique_ptr<cpio_entry_base> entry;
    while (pos < sz) {
        header = (cpio_newc_header *)(buf + pos);
        entry = make_unique<cpio_entry_base>(header);
        pos += sizeof(*header);
        string_view name_view(buf + pos);
        pos += x8u(header->namesize);
        pos_align(pos);
        if (name_view == "." || name_view == "..")
            continue;
        if (name_view == "TRAILER!!!")
            break;
        entry->data = buf + pos;
        pos += entry->filesize;
        entries[name_view] = std::move(entry);
        pos_align(pos);
    }
}

cpio_mmap::~cpio_mmap() {
    munmap(buf, sz);
}
