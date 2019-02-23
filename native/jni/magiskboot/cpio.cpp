#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <memory>
#include <utils.h>
#include <logging.h>

#include "cpio.h"

using namespace std;

static uint32_t x8u(char *hex) {
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

#define parse_align() lseek(fd, do_align(lseek(fd, 0, SEEK_CUR), 4), SEEK_SET)
cpio_entry::cpio_entry(int fd, cpio_newc_header &header)
: mode(x8u(header.mode)), uid(x8u(header.uid)), gid(x8u(header.gid)),
filesize(x8u(header.filesize)) {
	uint32_t namesize = x8u(header.namesize);
	filename.resize(namesize - 1);
	xxread(fd, &filename[0], namesize);
	parse_align();
	if (filesize) {
		data = xmalloc(filesize);
		xxread(fd, data, filesize);
		parse_align();
	}
}

cpio_entry::~cpio_entry() {
	free(data);
}

cpio::cpio(const char *filename) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0) return;
	fprintf(stderr, "Loading cpio: [%s]\n", filename);
	cpio_newc_header header;
	cpio_entry *entry = nullptr;
	while(xxread(fd, &header, sizeof(cpio_newc_header)) != -1) {
		entry = new cpio_entry(fd, header);
		if (entry->filename == "." || entry->filename == "..")
			continue;
		if (entry->filename == "TRAILER!!!")
			break;
		entries[entry->filename].reset(entry);
		entry = nullptr;
	}
	delete entry;
	close(fd);
}

#define dump_align() write_zero(fd, align_off(lseek(fd, 0, SEEK_CUR), 4))
void cpio::dump(const char *file) {
	fprintf(stderr, "Dump cpio: [%s]\n", file);
	unsigned inode = 300000;
	char header[111];
	int fd = creat(file, 0644);
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
		xwrite(fd, header, 110);
		xwrite(fd, e.first.data(), e.first.size() + 1);
		dump_align();
		if (e.second->filesize) {
			xwrite(fd, e.second->data, e.second->filesize);
			dump_align();
		}
	}
	// Write trailer
	sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
			inode++, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 11, 0);
	xwrite(fd, header, 110);
	xwrite(fd, "TRAILER!!!\0", 11);
	dump_align();
	close(fd);
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

void cpio::insert(cpio_entry *e) {
	auto ex = entries.extract(e->filename);
	if (!ex) {
		entries[e->filename].reset(e);
	} else {
		ex.key() = e->filename;
		ex.mapped().reset(e);
		entries.insert(std::move(ex));
	}
}

void cpio::add(mode_t mode, const char *name, const char *file) {
	void *buf;
	size_t sz;
	mmap_ro(file, &buf, &sz);
	auto e = new cpio_entry(name);
	e->mode = S_IFREG | mode;
	e->filesize = sz;
	e->data = xmalloc(sz);
	memcpy(e->data, buf, sz);
	munmap(buf, sz);
	insert(e);
	fprintf(stderr, "Add entry [%s] (%04o)\n", name, mode);
}

void cpio::makedir(mode_t mode, const char *name) {
	auto e = new cpio_entry(name);
	e->mode = S_IFDIR | mode;
	insert(e);
	fprintf(stderr, "Create directory [%s] (%04o)\n", name, mode);
}

void cpio::ln(const char *target, const char *name) {
	auto e = new cpio_entry(name);
	e->mode = S_IFLNK;
	e->filesize = strlen(target);
	e->data = strdup(target);
	insert(e);
	fprintf(stderr, "Create symlink [%s] -> [%s]\n", name, target);
}

void cpio::mv(entry_map::iterator &it, const char *to) {
	fprintf(stderr, "Move [%s] -> [%s]\n", it->first.data(), to);
	auto ex = entries.extract(it);
	ex.mapped()->filename = to;
	ex.key() = ex.mapped()->filename;
	entries.insert(std::move(ex));
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
