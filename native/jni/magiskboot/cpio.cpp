#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "cpio.h"
#include "utils.h"
#include "logging.h"

#define parse_align() lseek(fd, align(lseek(fd, 0, SEEK_CUR), 4), SEEK_SET)

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

cpio_entry::cpio_entry(int fd, cpio_newc_header &header) {
	// ino = x8u(header.ino);
	mode = x8u(header.mode);
	uid = x8u(header.uid);
	gid = x8u(header.gid);
	// nlink = x8u(header.nlink);
	// mtime = x8u(header.mtime);
	filesize = x8u(header.filesize);
	// devmajor = x8u(header.devmajor);
	// devminor = x8u(header.devminor);
	// rdevmajor = x8u(header.rdevmajor);
	// rdevminor = x8u(header.rdevminor);
	uint32_t namesize = x8u(header.namesize);
	// check = x8u(header.check);
	filename = CharArray(namesize);
	xxread(fd, filename, filename.size());
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

// Define the way to sort cpio_entry
template<>
int(*Vector<cpio_entry*>::_cmp)(cpio_entry*&, cpio_entry*&) = [](auto a, auto b) -> int {
	if (a == b) return 0;
	if (a == nullptr) return 1;
	if (b == nullptr) return -1;
	return a->filename.compare(b->filename);
};


cpio::cpio(const char *filename) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0) return;
	fprintf(stderr, "Loading cpio: [%s]\n", filename);
	cpio_newc_header header;
	cpio_entry *entry;
	while(xxread(fd, &header, sizeof(cpio_newc_header)) != -1) {
		entry = new cpio_entry(fd, header);
		if (entry->filename == "." || entry->filename == ".." || entry->filename == "TRAILER!!!") {
			bool trailer = entry->filename[0] == 'T';
			delete entry;
			if (trailer)
				break;
		}
		arr.push_back(entry);
	}
	close(fd);
}

cpio::~cpio() {
	for (auto &e : arr)
		if (e) delete e;
}

#define dump_align() write_zero(fd, align_off(lseek(fd, 0, SEEK_CUR), 4))
void cpio::dump(const char *file) {
	fprintf(stderr, "Dump cpio: [%s]\n", file);
	unsigned inode = 300000;
	char header[111];
	sort();
	int fd = creat(file, 0644);
	for (auto &e : arr) {
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
				(uint32_t) e->filename.size(),
				0           // e->check
		);
		xwrite(fd, header, 110);
		xwrite(fd, e->filename, e->filename.size());
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
	for (int i = 0; i < arr.size(); ++i) {
		if (!arr[i])
			continue;
		if (arr[i]->filename == name)
			return i;
	}
	return -1;
}

void cpio::insert(cpio_entry *e) {
	int i = find(e->filename);
	if (i >= 0) {
		delete arr[i];
		arr[i] = e;
	} else {
		arr.push_back(e);
	}
}

void cpio::insert(Vector<cpio_entry *> &arr) {
	for (auto &e : arr)
		insert(e);
}

void cpio::rm(const char *name, bool r) {
	size_t len = strlen(name);
	for (auto &e : arr) {
		if (!e)
			continue;
		if (e->filename.compare(name, len) == 0 &&
			((r && e->filename[len] == '/') || e->filename[len] == '\0')) {
			fprintf(stderr, "Remove [%s]\n", e->filename.c_str());
			delete e;
			e = nullptr;
			if (!r)
				return;
		}
	}
}

void cpio::makedir(mode_t mode, const char *name) {
	auto e = new cpio_entry();
	e->mode = S_IFDIR | mode;
	e->filename = name;
	insert(e);
	fprintf(stderr, "Create directory [%s] (%04o)\n", name, mode);
}

void cpio::ln(const char *target, const char *name) {
	auto e = new cpio_entry();
	e->mode = S_IFLNK;
	e->filename = name;
	e->filesize = strlen(target);
	e->data = strdup(target);
	insert(e);
	fprintf(stderr, "Create symlink [%s] -> [%s]\n", name, target);
}

void cpio::add(mode_t mode, const char *name, const char *file) {
	int fd = xopen(file, O_RDONLY);
	auto e = new cpio_entry();
	e->mode = S_IFREG | mode;
	e->filename = name;
	e->filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	e->data = xmalloc(e->filesize);
	xxread(fd, e->data, e->filesize);
	close(fd);
	insert(e);
	fprintf(stderr, "Add entry [%s] (%04o)\n", name, mode);
}

bool cpio::mv(const char *from, const char *to) {
	int f = find(from), t = find(to);
	if (f >= 0) {
		if (t > 0) {
			delete arr[t];
			arr[t] = nullptr;
		}
		fprintf(stderr, "Move [%s] -> [%s]\n", from, to);
		arr[f]->filename = to;
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
	for (auto &e : arr) {
		if (!e)
			continue;
		extract_entry(e, e->filename);
	}
}

bool cpio::extract(const char *name, const char *file) {
	int i = find(name);
	if (i > 0) {
		auto e = arr[i];
		extract_entry(e, file);
		return true;
	}
	fprintf(stderr, "Cannot find the file entry [%s]\n", name);
	return false;
}

void cpio::sort() {
	arr.sort();
	while (arr.back() == nullptr)
		arr.pop_back();
}
