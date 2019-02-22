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

#define parse_align() lseek(fd, do_align(lseek(fd, 0, SEEK_CUR), 4), SEEK_SET)

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
	unique_ptr<cpio_entry> entry;
	while(xxread(fd, &header, sizeof(cpio_newc_header)) != -1) {
		entry = std::make_unique<cpio_entry>(fd, header);
		if (entry->filename == "." || entry->filename == "..")
			continue;
		if (entry->filename == "TRAILER!!!")
			break;
		entries.push_back(std::move(entry));
	}
	close(fd);
}

#define dump_align() write_zero(fd, align_off(lseek(fd, 0, SEEK_CUR), 4))
void cpio::dump(const char *file) {
	fprintf(stderr, "Dump cpio: [%s]\n", file);
	unsigned inode = 300000;
	char header[111];
	sort();
	int fd = creat(file, 0644);
	for (auto &e : entries) {
		sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
				inode++,    // e->ino
				e->mode,
				e->uid,
				e->gid,
				1,          // e->nlink
				0,          // e->mtime
				e->filesize,
				0,          // e->devmajor
				0,          // e->devminor
				0,          // e->rdevmajor
				0,          // e->rdevminor
				(uint32_t) e->filename.size() + 1,
				0           // e->check
		);
		xwrite(fd, header, 110);
		xwrite(fd, e->filename.c_str(), e->filename.size() + 1);
		dump_align();
		if (e->filesize) {
			xwrite(fd, e->data, e->filesize);
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

int cpio::find(const char *name) {
	for (int i = 0; i < entries.size(); ++i) {
		if (!entries[i])
			continue;
		if (entries[i]->filename == name)
			return i;
	}
	return -1;
}

void cpio::insert(cpio_entry *e) {
	auto ue = unique_ptr<cpio_entry>(e);
	insert(ue);
}

void cpio::insert(unique_ptr<cpio_entry> &e) {
	int i = find(e->filename.c_str());
	if (i >= 0) {
		entries[i] = std::move(e);
	} else {
		entries.push_back(std::move(e));
	}
}

void cpio::rm(const char *name, bool r) {
	size_t len = strlen(name);
	for (auto &e : entries) {
		if (!e)
			continue;
		if (e->filename.compare(0, len, name) == 0 &&
			((r && e->filename[len] == '/') || e->filename[len] == '\0')) {
			fprintf(stderr, "Remove [%s]\n", e->filename.c_str());
			e.reset();
			if (!r) return;
		}
	}
}

void cpio::makedir(mode_t mode, const char *name) {
	auto e = make_unique<cpio_entry>(name);
	e->mode = S_IFDIR | mode;
	insert(e);
	fprintf(stderr, "Create directory [%s] (%04o)\n", name, mode);
}

void cpio::ln(const char *target, const char *name) {
	auto e = make_unique<cpio_entry>(name);
	e->mode = S_IFLNK;
	e->filesize = strlen(target);
	e->data = strdup(target);
	insert(e);
	fprintf(stderr, "Create symlink [%s] -> [%s]\n", name, target);
}

void cpio::add(mode_t mode, const char *name, const char *file) {
	void *buf;
	size_t sz;
	mmap_ro(file, &buf, &sz);
	auto e = make_unique<cpio_entry>(name);
	e->mode = S_IFREG | mode;
	e->filesize = sz;
	e->data = xmalloc(sz);
	memcpy(e->data, buf, sz);
	munmap(buf, sz);
	insert(e);
	fprintf(stderr, "Add entry [%s] (%04o)\n", name, mode);
}

bool cpio::mv(const char *from, const char *to) {
	int f = find(from), t = find(to);
	if (f >= 0) {
		if (t > 0)
			entries[t].reset();
		fprintf(stderr, "Move [%s] -> [%s]\n", from, to);
		entries[f]->filename = to;
		return true;
	}
	fprintf(stderr, "Cannot find entry %s\n", from);
	return false;
}

static void extract_entry(cpio_entry *e, const char *file) {
	fprintf(stderr, "Extract [%s] to [%s]\n", e->filename.c_str(), file);
	unlink(file);
	rmdir(file);
	if (S_ISDIR(e->mode)) {
		mkdir(file, e->mode & 0777);
	} else if (S_ISREG(e->mode)) {
		int fd = creat(file, e->mode & 0777);
		xwrite(fd, e->data, e->filesize);
		fchown(fd, e->uid, e->gid);
		close(fd);
	} else if (S_ISLNK(e->mode)) {
		auto target = new char[e->filesize + 1];
		memcpy(target, e->data, e->filesize);
		target[e->filesize] = '\0';
		symlink(target, file);
		delete[] target;
	}
}

void cpio::extract() {
	for (auto &e : entries) {
		if (!e) continue;
		extract_entry(e.get(), e->filename.c_str());
	}
}

bool cpio::extract(const char *name, const char *file) {
	int i = find(name);
	if (i > 0) {
		extract_entry(entries[i].get(), file);
		return true;
	}
	fprintf(stderr, "Cannot find the file entry [%s]\n", name);
	return false;
}

void cpio::sort() {
	std::sort(entries.begin(), entries.end(), []
	(const unique_ptr<cpio_entry> &a, const unique_ptr<cpio_entry> &b) -> bool {
		if (a == b || !a)
			return false;
		if (!b)
			return true;
		return a->filename < b->filename;
	});

	while (!entries.back())
		entries.pop_back();
}
